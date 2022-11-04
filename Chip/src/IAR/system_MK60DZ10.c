#include "misc.h"
#include "common.h"
#include "system_MK60DZ10.h"

extern void main(void);

/* close watch dog */
static void wdogDisable(void)
{
    /* 配置看门狗之前需要关闭看门狗 */
    uint8 tmp = __get_BASEPRI();//1表示关中断，0表示开中断
    
    /* 关中断写入数据，解锁看门狗 */
    DisableInterrupts;
    
    WDOG_UNLOCK = 0XC520;
    WDOG_UNLOCK = 0XD928;
    
    if(tmp == 0)
    {
        EnableInterrupts;
    }
    
    /* //WDOGEN清0，禁用看门狗 */
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;  
}

/* 复制中断向量表和必要的数据到 RAM 里 */
#pragma section = ".data"
#pragma section = ".data_init"
#pragma section = ".bss"
#pragma section = "CodeRelocate"
#pragma section = "CodeRelocateRam"

static void commonStartup(void)
{
    /* Declare a counter we'll use in all of the copy loops */
    uint32 n;

    /* Declare pointers for various data sections. These pointers
     * are initialized using values pulled in from the linker file
     */
    uint8 *data_ram, * data_rom, * data_rom_end;
    uint8 *bss_start, * bss_end;


    /*  VECTOR_TABLE 和 VECTOR_RAM 的地址从  linker 文件里获得 （*.icf） */
    extern uint32 __VECTOR_TABLE[];
    extern uint32 __VECTOR_RAM[];

    /* 复制ROM里的中断向量表到RAM里 */
    if (__VECTOR_RAM != __VECTOR_TABLE)             //如果不是RAM启动，则需要复制中断向量表
    {
        for (n = 0; n < 0x410; n++)                 //逐个复制
            __VECTOR_RAM[n] = __VECTOR_TABLE[n];
    }
    
    /* 指定新的中断向量表地址为 __VECTOR_RAM */
    write_vtor((uint32)__VECTOR_RAM);

    /* 把已赋初值的变量从ROM里复制数据到RAM里 */
    data_ram = __section_begin(".data");            //已赋初值的变量的地址在RAM里
    data_rom = __section_begin(".data_init");       //已赋初值的变量的数据存放在ROM里，需要赋值到RAM里
    data_rom_end = __section_end(".data_init");
    n = data_rom_end - data_rom;

    /* 复制初始化数据到RAM里 */
    while (n--)
        *data_ram++ = *data_rom++;

    /* 没赋初值或者初值为0的变量，需要清除其RAM里的数据，确保值为0 */
    bss_start = __section_begin(".bss");
    bss_end = __section_end(".bss");

    /* 清除没赋初值或者初值为0的变量数据值 */
    n = bss_end - bss_start;
    while(n--)
        *bss_start++ = 0;

    /* 赋值用 __ramfunc 声明的函数的的代码段到 RAM，可以加快代码的运行        */
    uint8 *code_relocate_ram = __section_begin("CodeRelocateRam");
    uint8 *code_relocate = __section_begin("CodeRelocate");
    uint8 *code_relocate_end = __section_end("CodeRelocate");

    /* 从ROM里复制函数代码到RAM里 */
    n = code_relocate_end - code_relocate;
    while (n--)
        *code_relocate_ram++ = *code_relocate++;
}

void start(void)
{
    wdogDisable();
    commonStartup();    // 复制中断向量表 和 必要的数据到 RAM里
    main();             // 执行用户主函数
    while(1);          // 死循环
}

