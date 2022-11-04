#include "misc.h"
#include "common.h"
#include "system_MK60DZ10.h"


int core_clk_khz;
int core_clk_mhz;
int bus_clk_khz;

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

/*!
 *  @brief      时钟分频设置函数
 *  @param      outdiv1    内核分频系数，       core    clk = MCG / (outdiv1 +1)
 *  @param      outdiv2    bus分频系数，        bus     clk = MCG / (outdiv2 +1)
 *  @param      outdiv3    flexbus分频系数，    flexbus clk = MCG / (outdiv3 +1)
 *  @param      outdiv4    flash分频系数，      flash   clk = MCG / (outdiv4 +1)
 *  @since      v1.0
 *  @author     飞思卡尔公司
 *  Sample usage:       set_sys_dividers(0,1, 9,3);     // core clk = MCG ; bus clk = MCG / 2 ; flexbus clk = MCG /10 ; flash clk = MCG / 4;
 */

static void set_sys_dividers(uint32 outdiv1, uint32 outdiv2, uint32 outdiv3, uint32 outdiv4)
{
    /*
    * This routine must be placed in RAM. It is a workaround for errata e2448.
    * Flash prefetch must be disabled when the flash clock divider is changed.
    * This cannot be performed while executing out of flash.
    * There must be a short delay after the clock dividers are changed before prefetch
    * can be re-enabled.
    */
    uint32 temp_reg;
    uint8 i;

    temp_reg = FMC_PFAPR; // store present value of FMC_PFAPR

    // set M0PFD through M7PFD to 1 to disable prefetch
    FMC_PFAPR |= FMC_PFAPR_M7PFD_MASK | FMC_PFAPR_M6PFD_MASK | FMC_PFAPR_M5PFD_MASK
                 | FMC_PFAPR_M4PFD_MASK | FMC_PFAPR_M3PFD_MASK | FMC_PFAPR_M2PFD_MASK
                 | FMC_PFAPR_M1PFD_MASK | FMC_PFAPR_M0PFD_MASK;

    // set clock dividers to desired value
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(outdiv1) | SIM_CLKDIV1_OUTDIV2(outdiv2)
                  | SIM_CLKDIV1_OUTDIV3(outdiv3) | SIM_CLKDIV1_OUTDIV4(outdiv4);

    // wait for dividers to change
    for (i = 0 ; i < outdiv4 ; i++)
        {}

    FMC_PFAPR = temp_reg; // re-store original value of FMC_PFAPR

    return;
}

static void pllInit(void)
{
    //上电复位后，单片机会自动进入 FEI 模式，使用 内部参考时钟

    //FEI -> FBE
    MCG_C2 &= ~MCG_C2_LP_MASK;
    MCG_C2 |= MCG_C2_RANGE(1);
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(7); 
    while (MCG_S & MCG_S_IREFST_MASK) {};                                //等待FLL参考时钟 为 外部参考时钟（S[IREFST]=0，表示使用外部参考时钟，）
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {}; //等待选择外部参考时钟
    //现在已经进入了 FBE模式

    //FBE -> PBE
    set_sys_dividers(0, 1, 9, 3); //设置系统分频因子选项
    MCG_C5 = MCG_C5_PRDIV(4);                       //分频， EXTAL_IN_MHz/( PRDIV+1)
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(20);    //倍频， EXTAL_IN_MHz/( PRDIV+1)  * (VDIV+24)
    while (!(MCG_S & MCG_S_PLLST_MASK)) {};         //等待时钟源选择PLL
    while (!(MCG_S & MCG_S_LOCK_MASK)) {};          //等待 PLL锁了（锁相环）
    // 现在已经进入了 PBE 模式
    
    // PBE -> PEE
    MCG_C1 &= ~MCG_C1_CLKS_MASK;
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};//等待选择输出PLL
    // 现在已经进入了 PEE 模式
}

void start(void)
{
    wdogDisable();
    commonStartup();    // 复制中断向量表 和 必要的数据到 RAM里
    //pllInit();          // 系统初始化，设置系统频率，初始化printf端口
    main();             // 执行用户主函数
    while(1);          // 死循环
}

