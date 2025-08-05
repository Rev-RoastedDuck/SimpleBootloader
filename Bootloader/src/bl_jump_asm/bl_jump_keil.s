    AREA    |.text|, CODE, READONLY
    THUMB

    EXPORT  __bootloader_jump_to_app_keil

__bootloader_jump_to_app_keil
    ; R0 = 应用程序地址

    LDR     R1, [R0, #0]     ; 加载栈顶地址
    MSR     MSP, R1          ; 设置主堆栈指针
    LDR     R1, [R0, #4]     ; 加载复位地址 R1 = R0 + 4
    DSB                      ; 数据同步
    ISB                      ; 指令同步
    BX      R1               ; 跳转到 APP (R1)

    END
