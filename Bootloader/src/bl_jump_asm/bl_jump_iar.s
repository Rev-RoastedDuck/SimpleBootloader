    .syntax unified
    .thumb

    PUBLIC __bootloader_jump_to_app_iar

__bootloader_jump_to_app_iar:
    MSR MSP, r0          ; 设置主堆栈指针
    LDR r1, [r0, #4]     ; 取复位地址
    DSB                  ; 数据同步屏障
    ISB                  ; 指令同步屏障
    BX  r1               ; 跳转到复位地址
    BX  lr               ; 保险跳转