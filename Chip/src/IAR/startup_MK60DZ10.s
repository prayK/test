    SECTION .noinit : CODE          ; //指定代码段：.noinit
    EXPORT  Reset_Handler            ; //定义 Reset_Handler 函数
Reset_Handler
    CPSIE   i                        ; //使能全部中断
    import start                     ; //声明函数
    BL      start                    ; //调用 C语言函数 start
__done
    B       __done


        END
