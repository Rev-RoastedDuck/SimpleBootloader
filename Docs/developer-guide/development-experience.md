## 实现
### **1. 双应用区（A/B）架构设计**
- **独立应用管理**：每个应用区具有独立的固件信息结构（bl_main.c），包括地址、大小、版本、CRC32 校验等
- **平滑升级与回退**：升级后可自动回退到前一个应用，无需手动干预
- **自动完整性校验**：启动前自动校验 CRC32，防止固件损坏导致的启动失败

### **2. 通信协议体系**
- **内置串口协议**：支持 XMODEM/YMODEM，包括：
  - 128/1K 数据包自适应
  - CRC16/SUM 多种校验方式
  - 智能失败重传机制
- **易于扩展**：通过统一的设备驱动接口，支持 CAN、USB、以太网等多种通信方式
- **DMA 高效缓冲**：使用 DMA 环形队列 实现高效的数据缓冲和传输
- **多设备支持**：同时扫描多个通信设备

### **3. 调试系统**
DEBUG 模块功能丰富：
- **彩色日志输出**：支持不同日志级别（INFO、DEBUG、ERROR、WARNING、SUCCESS 等）
- **性能监测**：内置执行时间测量宏（`TIME_TAKEN_START`/`TIME_TAKEN_END`）
- **灵活重定向**：调试输出可重定向到串口、半主机等多种通道
- **断言支持**：便于开发调试和问题诊断

### **4. 超时保护机制**
- **多级超时管理**：bl_main.c 独立管理操作超时和下载超时
  - 操作超时：10 秒（防止无限等待）
  - 下载超时：30 秒（给予充足的传输时间）
- **可靠的状态恢复**：超时触发自动重置，避免系统卡死

### **5. 多编译器支持**
- **编译器兼容性**：支持 Keil MDK(ARMCC)、GNUC 和 IAR(ICCARM)
- **汇编适配**：多编译器跳转实现 确保可靠的应用跳转
  - `bl_jump_gcc.s`：GCC 编译器版本
  - `bl_jump_iar.s`：IAR 编译器版本
  - `bl_jump_keil.s`：Keil MDK 版本

### **6. 数据结构设计**
- **环形队列**：DMA_CIRCULAR_QUEUE_RRD 支持任意数据类型，内存连续，DMA 友好
- **零拷贝遍历**：提供高效的宏遍历接口 `dma_circular_queue_for_each`，降低 CPU 占用

## 执行流程

### **第一阶段：早期启动（Pre-Main）**

#### bl_main.c
```c
BL_CONSTRUCTOR_ATTRIBUTE
void bootloader_pre_main(void) {
  if(BL_JUMP_APP_FLAG_MAGIC_RRD == bl_app_jump_flag){
    bl_app_jump_flag = 0;
    bootloader_jump_to_app();
  }
}
```

**用途**：
- 检查 NO_INIT 段的魔数 `0xDEADBEEF`（在上次重启时设置）
- 若有魔数则立即跳转，无需执行完整的 bootloader 初始化
- 实现快速 APP 

### **第二阶段：主程序初始化**

#### bl_main.c

**初始化步骤**：

1. **平台设备初始化**
   ```c
   bl_platform_device_init();  // 初始化 UART、Flash、DMA 等硬件
   ```

2. **CRC32 查表初始化**
   ```c
   bl_firware_crc32_table_init();  // 分配 256×4 字节内存的 CRC 表
   ```

3. **打印配置信息**（用于调试）
   - 超时配置表
   - 命令魔数表

4. **应用管理器初始化**
   ```c
   bl_apps_manager_new();        // 从堆分配应用管理器
   bl_apps_manager_load_info();  // 从 Flash 读取应用信息
   ```
   - 验证应用管理器魔数
   - CRC32 校验所有应用固件
   - 自动清除损坏的应用信息

5. **主循环**
   ```c
   bootloader_upgrade_app();  // 进入命令处理循环
   ```

### **第三阶段：主循环（Command Loop）**

#### **状态定义**
```c
typedef enum __BL_STATUS{
    BL_STATUS_WAIT_FOR_CMD = 0,   // 等待命令
    BL_STATUS_UPGRADE,             // 升级中
}BL_STATUS;
```

#### **命令定义**
```c
typedef enum __BL_CMD{
    BL_CMD_NONE = 0,
    BL_CMD_UPGRADE,                // 升级固件：0xFEDCBA98
    BL_CMD_SWITCH_APP,             // 切换应用：0x12345678
    BL_CMD_RESET_APP_MANAGER,      // 重置应用管理器：0x87654321
}BL_CMD;
```

#### **详细流程**

##### **阶段 1：等待命令（BL_STATUS_WAIT_FOR_CMD）**

```
扫描所有已注册的通信设备缓冲区
    ↓
在缓冲区中查找 4 字节魔数
    ↓
[找到] → 识别命令类型
    ↓ [重置操作超时计时器]
    ↓
执行对应命令：
  • UPGRADE: 切换到升级缓冲区，等待固件数据
  • SWITCH_APP: 切换下一个应用区，设置跳转标志
  • RESET_APP_MANAGER: 重置应用管理器数据
[未找到] → 继续扫描，检查超时
```
##### **阶段 2：升级处理（BL_STATUS_UPGRADE）**

**升级缓冲区初始化**：
```c
DMA_CIRCULAR_QUEUE_RRD upgrade_buff;
dma_circular_queue_init(upgrade_buff_ptr, 256, sizeof(uint8_t), 256);
```
- 创建 256 字节的环形缓冲区
- 用于临时存储接收到的数据包

**升级流程**：

```
状态：WAIT_FOR_CMD
    ↓
收到 UPGRADE 命令
    ↓
获取下一个应用区地址
    ↓
切换通信缓冲区为升级缓冲区
    ↓
状态 → UPGRADE
    ↓
循环接收固件数据包：
    
    [1] xmodem.interface->start() 处理接收
    [2] 缓冲区有数据 && 长度有效
        ↓
        将缓冲区数据拷贝到临时数组
        ↓
        xmodem.interface->unpack() 解析数据包
        ↓
        [成功] → modem_save_data() 写入 Flash
        [完成] → 计算固件 CRC32，更新应用信息
    [3] 检查下载超时（30秒）
        ↓ [超时] → 退出升级，跳转到已加载的应用
```

##### **阶段 3：升级完成处理**
```c
static inline void bl_apps_manager_upgrade_success(bl_apps_manager_rrd *self, uint32_t finished_addr){
    bl_app_info_rrd *app = &self->apps[self->download_app_index];
    
    app->valid_firware = true;                          // 标记为有效
    app->app_size = finished_addr - app->app_addr;       // 计算大小
    app->app_crc32 = bl_firware_crc32_compute(          // 计算 CRC32
        (const uint8_t *)(uintptr_t)app->app_addr, 
        app->app_size);
    
    self->upgrade_flag = 0;                              // 清除升级标志
    self->download_app_count++;                          // 升级计数
    self->download_app_index = (self->download_app_index + 1) % BL_APPLICATION_NUMBER;  // 轮转
    
    // 写回 Flash 保存元数据
    g_bl_flash->interface->write_pre(g_bl_flash, BL_FRIWARE_INFO_ADDRESS);
    g_bl_flash->interface->write(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));
}
```

### **第四阶段：应用跳转**

#### **超时自动跳转**
```c
if(bl_apps_manager_have_firware(g_bl_apps_manager_rrd) && bl_operate_timeout()){
    // 10 秒无操作 + 已有有效固件 → 自动跳转
    goto bl_operate_timeoutcode;
}
```

#### **跳转前清理**
```c
bl_operate_timeoutcode:
    upgrade_buff_ptr->del((void **)&upgrade_buff_ptr);  // 释放缓冲区
    DEBUG_PRINT_INFO(1, "[bootloader] launch app%d...", bl_apps_manager_get_app_index(g_bl_apps_manager_rrd));
    bootloader_jump_to_app_pre();  // 准备跳转
```

#### **跳转**
```c
static inline void bootloader_jump_to_app(void){
    bl_platform_disable_irq();  // 禁用中断
    #ifdef SCB
        SCB->VTOR = bl_app_jump_address;  // 更新中断向量表基址
    #endif
    bl_platform_enable_irq();   // 恢复中断
    __bootloader_jump_to_app(bl_app_jump_address);  // 汇编跳转
}
```

**跳转顺序**：
1. 关闭所有中断（防止中断打扰）
2. 修改 VTOR 寄存器（ARM Cortex-M 中断向量表基址）
3. 设置堆栈指针为 APP 入口的第一个 32 位值
4. 跳转到 APP 复位向量（第二个 32 位值）

#### **多编译器跳转实现**
```c
#if defined(__CC_ARM)
    void __bootloader_jump_to_app_keil(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_keil
#elif defined(__GNUC__)
    void __bootloader_jump_to_app_gcc(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_gcc
#elif defined(__ICCARM__)
    void __bootloader_jump_to_app_iar(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_iar
#endif
```
### **完整时序图**
```
┌─────────────────────────────────────────────────────────────┐
│                      系统启动（上电）                          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│           bootloader_pre_main()  [构造器自动执行]              │
│     检查 bl_app_jump_flag 魔数（0xDEADBEEF）                 │
└─────────────────────────────────────────────────────────────┘
                     ↙ 有魔数    ↖ 无魔数
             [直接跳转APP]    [继续初始化]
                             ↓
    ┌───────────────────────────────────────────────────┐
    │      bootloader_main() 初始化                       │
    │  1. 硬件初始化（UART、Flash、DMA）                 │
    │  2. CRC32 查表初始化                               │
    │  3. 加载应用管理信息（验证和校验）                 │
    └───────────────────────────────────────────────────┘
                            ↓
    ┌───────────────────────────────────────────────────┐
    │      bootloader_upgrade_app() 主循环               │
    │  状态机：等待命令 ↔ 升级中                         │
    └───────────────────────────────────────────────────┘
        ↓
    ┌─ 等待命令状态（WAIT_FOR_CMD）─────────────────┐
    │                                                 │
    │  ┌─ 扫描所有通信设备缓冲区                     │
    │  │  [10秒无操作] ──→ [自动跳转APP]            │
    │  │  [接收到魔数] ──→ 识别并执行命令              │
    │  │  ┌─ UPGRADE ────→ 进入升级状态              │
    │  │  ├─ SWITCH_APP ─→ 切换应用区                │
    │  │  └─ RESET ─────→ 重置应用管理器            │
    │  └─ 循环扫描                                   │
    │                                                 │
    └─────────────────────────────────────────────────┘
        ↓
    ┌─ 升级状态（UPGRADE）─────────────────────────┐
    │  [XMODEM 接收器启动]                          │
    │  ┌─ 接收固件数据包（128/1K）                  │
    │  │  [数据有效] ──→ 写入 Flash                  │
    │  │  [包完成] ──→ 发送 ACK                     │
    │  │  [包错误] ──→ 请求重传（最多10次）        │
    │  │  [30秒超时] ──→ 退出升级                   │
    │  │  [升级完成] ──→ 计算 CRC32、保存元数据      │
    │  └─ 循环处理                                  │
    │                                                 │
    └─────────────────────────────────────────────────┘
        ↓
    ┌───────────────────────────────────────────────────┐
    │      跳转前准备                                    │
    │  1. 释放所有动态分配的内存                        │
    │  2. 设置 bl_app_jump_flag = 0xDEADBEEF            │
    │  3. 设置 bl_app_jump_address = APP 地址           │
    │  4. 系统软复位                                    │
    └───────────────────────────────────────────────────┘
        ↓
    ┌───────────────────────────────────────────────────┐
    │   复位发生，重新启动 bootloader                    │
    │   bootloader_pre_main() 检测到魔数                │
    └───────────────────────────────────────────────────┘
        ↓
    ┌───────────────────────────────────────────────────┐
    │  bootloader_jump_to_app() [真实跳转]              │
    │  1. 禁用中断                                      │
    │  2. SCB->VTOR = APP 地址                          │
    │  3. 恢复中断                                      │
    │  4. 设置堆栈指针为 APP 入口第一个 32 位            │
    │  5. 跳转到 APP 复位向量（第二个 32 位）            │
    └───────────────────────────────────────────────────┘
        ↓
    ┌─────────────────────────────────────────────────────────────┐
    │                   应用程序运行                               │
    └─────────────────────────────────────────────────────────────┘
```

## 开发思路
### APP跳转
1. 启动时检查 `bl_app_jump_flag`，有魔数标志则跳转APP
2. 跳转APP流程：关闭中断、修改中断向量表、开启中断、设置堆栈指针、获取复位地址、通过复位转到APP
3. 启动后，bootloader会计算已加载的固件的crc32校验码，然后在主循环中检测已注册到表中的设备缓冲区，通过比较缓冲区的数据和预设的魔数，执行相关的命令，包括下载固件、切换app、初始化bootloader。如果一定时间内没有收到指令或接收到下载固件的指令但下载固件超时，bootloader会自动退出，跳转到已加载的app中，没有下载任何app则一直等待。

### 其他
- 为了减小 bootloader 的占用大小，本项目中内存占用较大的变量，均用malloc(...)从堆中分配内存，跳转到app前，用free(...)释放内存
- 为了增加可拓展性，接口和外设均采用注册的形式载入 bootloader
- 设备基类均面向数据实现，与实际外设无关联

## 新东西
- XModem
    - 数据包格式：SOH/STX、block_num、255-(block_num)、data(128/1K)、CRC-16/SUM

    | Sender发送方                     | 方向 | Receiver接收方 |
    | -------------------------------- | ---- | -------------- |
    | SOH、0x01、0xFE、data、chechsum  | →    | OK             |
    |                                  | ←    | ACK            |
    | SOH、0x02、0xFD、data、chechsum  | →    | OK             |
    |                                  | ←    | ACK            |
    | SOH、0x03、0xFC、data、chechsum  | →    | OK             |
    |                                  | ←    | ACK            |
    | SOH、0x04、0xFB、data、chechsum  | →    | OK             |
    |                                  | ←    | ACK            |
    | SOH、0x05、0xFA、data、chechsum  | →    | OK             |
    |                                  | ←    | ACK            |
    | EOT                              | →    | OK             |
    |                                  | ←    | ACK            |
- YModem
    - 一次连接可以传输多个数据包
    - 结束session时，发送方发送`SOH、0x00、0xFF、Data[128]={0}、CRC-16`

    | Sender发送方                          | 方向 | Receiver接收方 |
    | ------------------------------------- | ---- | -------------- |
    | SOH、0x00、0xFF、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x01、0xFE、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x02、0xFD、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x03、0xFC、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | EOT                                   | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x00、0xFF、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x01、0xFE、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x02、0xFD、Data[128]、CRC-16    | →    |                |
    |                                       | ←    | ACK            |
    | EOT                                   | →    |                |
    |                                       | ←    | ACK            |
    | SOH、0x00、0xFF、Data[128]={0}、CRC-16 | →    |                |
    |                                       | ←    | ACK            |
- 在 BSS段/ZI段 后面新增一个 NO_INIT 段，可以防止 NO_INIT 段的数据在上电后清零
- 使用复位可以百分百保证APP跳转成功，因为复位之后的环境是最干净的，不需要手动清理中断、DMA、定时器等，不同编译器和芯片都支持
- 内存分区: 代码段、数据段、BSS段、堆、栈
- gunc平台中，需要重写_write(...)来实现printf(...)重定向
- 数组宏配置
    ```
    #define BL_APPLICATION_ADDRESS_LIST {                           \
                                            (uint32_t)0x0800A800,   \
                                            (uint32_t)0x08014000,   \
                                        }
    static const uint32_t g_app_addr_list[] = BL_APPLICATION_ADDRESS_LIST;
    #define BL_APPLICATION_NUMBER  (sizeof((uint32_t[])BL_APPLICATION_ADDRESS_LIST) / sizeof(uint32_t))
    ```
- flash写入前，可以把数据拼接成8字节，然后按 字 写入，注意大小端
    ```
    for (size_t i = 0; i < length; i += 8) {
        uint64_t data_word = 0xFFFFFFFF;
        // 拼接8字节 小端
        for (uint8_t j = 0; j < 8 && (i + j) < length; j++) {
            ((uint8_t*)&data_word)[j] = buff[i + j];
        }
    }
    ```
- 如果一个缓冲区被绑定到DMA传输，在读写这个缓冲区的时候，需要关闭中断和DMA传输，保证数据在此时不会被修改
- malloc 字节对齐
    ```c
    inline void* aligned_malloc(size_t size, size_t alignment)
    {
        uintptr_t raw = (uintptr_t)malloc(size + alignment - 1 + sizeof(void*));
        if (raw == 0) return NULL;
        uintptr_t aligned = (raw + sizeof(void*) + alignment - 1) & ~(alignment - 1);
        ((void**)aligned)[-1] = (void*)raw;

        return (void*)aligned;
    }

    void aligned_free(void* aligned_ptr)
    {
        if (aligned_ptr) {
            free(((void**)aligned_ptr)[-1]);
        }
    }

    ```