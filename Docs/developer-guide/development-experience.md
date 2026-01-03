## 踩过的坑

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
- 使用复位可以百分百保证APP跳转成功，因为复位之后的环境是最干净的，不需要手动去初始化
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