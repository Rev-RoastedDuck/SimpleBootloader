    .syntax unified
    .thumb

    .global __bootloader_jump_to_app_gcc
    .type __bootloader_jump_to_app_gcc, %function

__bootloader_jump_to_app_gcc:
    msr msp, r0         // 设置主堆栈指针
    ldr r1, [r0, #4]    // 取复位地址
    dsb                 // 数据同步屏障
    isb                 // 指令同步屏障
    bx r1               // 跳转到复位地址
    bx lr               // 保险跳转