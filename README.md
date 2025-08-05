<div align="center">
  <img src="./Docs/images/icon.webp" height="150">
  <h1>SimpleBootloader</h1>
  <span>一个轻量级、模块化的 C 语言 Bootloader 方案</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-GPLv3-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>
<p align="center">
<a href="./Docs/README_en.md">English</a> | <a href="">简体中文</a>
</p>

## 项目简介
**SimpleBootloader** 是一个面向嵌入式系统的轻量级 C 语言 Bootloader，支持固件升级、应用跳转、通信协议扩展等功能，结构清晰，易于集成和二次开发。

<img src="./Docs/images/feature.png">


## 特性
- 支持双应用区（A/B 区）管理，升级后可回退。每个应用区具有独立的固件信息（地址、大小、版本、CRC32 校验），启动前自动校验固件完整性
- 内置 XMODEM/YMODEM 等主流串口升级协议，支持128/1K数据包、CRC16/SUM校验和失败重传，可扩展 CAN、USB、以太网等多种通信方式
- 内置多级调试输出接口，支持彩色日志、断言、性能统计等功能。调试输出可灵活重定向到串口、半主机等多种通道
- 具备灵活的超时保护机制保障机制
- 底层通信缓冲区采用高效 DMA 环形队列
- 代码模块化，易于裁剪和扩展

## 目录结构
- `Bootloader/`
  - `include/`：头文件及各功能模块接口
    - `AlgorithmSuite/` 算法相关模块
    - `DebugSuite/` 调试与日志模块
    - `ModemSuite/` 协议相关模块
  - `interface/` Bootloader 对外接口
  - `src/`：Bootloader 主体源码
    - `bl_devices/` 通信、Flash 等设备驱动
    - `bl_jump_asm/` 多编译器支持的跳转汇编实现
- `Docs/`：文档与说明

## 快速开始

### 1. 克隆仓库
```bash
git clone https://github.com/Rev-RoastedDuck/SimpleBootloader.git
cd SimpleBootloader
```

### 2. 配置项目文件
- 进入 `Bootloader/interface/bl_config.h`，根据实际 Flash 分区和应用需求，配置如下参数：
  - `BL_FLASH_BASE_ADDRESS`：Bootloader 起始地址
  - `BL_APPLICATION_ADDRESS_X`：主/备应用区起始地址
  - `BL_FRIWARE_INFO_ADDRESS`：固件信息存储区地址
  - `BL_APPLICATION_NUMBER`：应用区数量
  - `BL_TIMEOUT_NO_OPERATE_RRD`：无操作超时时间（单位：ms）
  - `BL_TIMEOUT_DOWNLOAD_RRD`：下载超时时间（单位：ms）
  - 其余 MAGIC 宏用于升级、切换、校验等流程的安全标记，通常无需修改

- 进入 `Bootloader/interface/bl_platform_config.h`，根据目标平台实现或适配以下接口：
  - `bl_platform_device_init` / `bl_platform_device_deinit`：设备初始化与反初始化
  - `bl_platform_system_reset`：系统复位
  - `bl_platform_enable_irq` / `bl_platform_disable_irq`：全局中断使能与关闭
  - `bl_platform_get_systick`：获取系统时间戳

- 进入 `Bootloader/src/bl_platform.h`，根据实际硬件平台和通信需求，完成通信接口和 Flash 设备的注册：
  - `bootloader_platform_register_comm(bl_comm_rrd *comm)` 注册所有需要支持的通信通道（如串口、CAN、USB 等）
  - `bootloader_platform_register_flash(bl_flash_rrd *flash)` 注册底层 Flash 设备驱动
  - 如需指定调试输出通道，可通过 `bootloader_platform_register_debug_com_dev(bl_comm_rrd *comm)` 注册调试通信设备

### 3. 配置 no_init 区域
- **Keil**
  -  需修改`scatter`文件（.sct）以配置`NO_INIT`区域：
   - 修改前：
        ```text
        LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
          ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
          *.o (RESET, +First)
          *(InRoot$$Sections)
          .ANY (+RO)
          .ANY (+XO)
          }
          RW_IRAM1 0x20000000 0x00000400  {
            .ANY (+RW +ZI)
          }
        }
        ```
  - 修改后：
      ```text
      LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
        ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
        *.o (RESET, +First)
        *(InRoot$$Sections)
        .ANY (+RO)
        .ANY (+XO)
        }
        RW_IRAM1 0x20000000 0x00007C00  {  ; 减小 RW 区间到 31KB
          .ANY (+RW +ZI)
        }
        RW_IRAM2 0x20007C00 UNINIT 0x00000400 { ; 最后1KB用于 NO_INIT
          .ANY (NO_INIT)
        }
      }
      ```

### 4. 烧录
- 将生成的 Bootloader 固件烧录至目标板 Flash 起始地址，按需配置应用程序地址
- 升级或烧录应用程序时，建议选用**部分擦除**

## 注意事项
- `Magic` 宏（如升级、切换、复位等命令）为**32位无符号整数**，在通信和存储时需保证**大小端一致**，采用**字节序表示**（即按内存实际字节顺序传输），**不是字符串**，避免因端序不一致导致命令识别失败
- 如果未启用 `BL_CONSTRUCTOR_ATTRIBUTE`（即没有 `__attribute__((constructor))`），**必须在主函数 `main()` 的最顶部手动调用 `bootloader_pre_main()`**，以确保跳转标志和启动流程正确执行
- 烧录 Bootloader 和应用程序时，务必确认各自的起始地址、大小和分区不重叠，避免覆盖
- 使用部分擦除功能升级应用时，切勿擦除 Bootloader 和固件信息区，防止系统无法启动或回退
- 若使用 NO_INIT 区域，需确保链接脚本或`scatter`文件正确分区，避免与正常 RAM避免与正常`RAM`区域重叠
- `Bootloader`和应用程序的起始地址必须以 Flash 页大小对齐，避免因未对齐导致擦写异常或启动失败。请根据芯片 Flash 页大小（如 STM32 常见为 2KB、4KB、8KB 等）合理设合理设置 `BL_FLASH_BASE_ADDRESS`、`BL_APPLICATION_ADDRESS_A/B` 等地址参数。

## 待办事项
- 提供图形化升级工具和上位机支持
- 增强断点续传与升级失败自动恢复机制
- 增加安全加密升级（如 AES、RSA 签名校验）功能

## 声明
该项目 **SimpleBootloader** 使用 Apache License 2.0 授权发布，具体内容请参阅 [**LICENSE**](./LICENSE)