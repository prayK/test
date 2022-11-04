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
    /* ���ÿ��Ź�֮ǰ��Ҫ�رտ��Ź� */
    uint8 tmp = __get_BASEPRI();//1��ʾ���жϣ�0��ʾ���ж�
    
    /* ���ж�д�����ݣ��������Ź� */
    DisableInterrupts;
    
    WDOG_UNLOCK = 0XC520;
    WDOG_UNLOCK = 0XD928;
    
    if(tmp == 0)
    {
        EnableInterrupts;
    }
    
    /* //WDOGEN��0�����ÿ��Ź� */
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;  
}

/* �����ж�������ͱ�Ҫ�����ݵ� RAM �� */
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


    /*  VECTOR_TABLE �� VECTOR_RAM �ĵ�ַ��  linker �ļ����� ��*.icf�� */
    extern uint32 __VECTOR_TABLE[];
    extern uint32 __VECTOR_RAM[];

    /* ����ROM����ж�������RAM�� */
    if (__VECTOR_RAM != __VECTOR_TABLE)             //�������RAM����������Ҫ�����ж�������
    {
        for (n = 0; n < 0x410; n++)                 //�������
            __VECTOR_RAM[n] = __VECTOR_TABLE[n];
    }
    
    /* ָ���µ��ж��������ַΪ __VECTOR_RAM */
    write_vtor((uint32)__VECTOR_RAM);

    /* ���Ѹ���ֵ�ı�����ROM�︴�����ݵ�RAM�� */
    data_ram = __section_begin(".data");            //�Ѹ���ֵ�ı����ĵ�ַ��RAM��
    data_rom = __section_begin(".data_init");       //�Ѹ���ֵ�ı��������ݴ����ROM���Ҫ��ֵ��RAM��
    data_rom_end = __section_end(".data_init");
    n = data_rom_end - data_rom;

    /* ���Ƴ�ʼ�����ݵ�RAM�� */
    while (n--)
        *data_ram++ = *data_rom++;

    /* û����ֵ���߳�ֵΪ0�ı�������Ҫ�����RAM������ݣ�ȷ��ֵΪ0 */
    bss_start = __section_begin(".bss");
    bss_end = __section_end(".bss");

    /* ���û����ֵ���߳�ֵΪ0�ı�������ֵ */
    n = bss_end - bss_start;
    while(n--)
        *bss_start++ = 0;

    /* ��ֵ�� __ramfunc �����ĺ����ĵĴ���ε� RAM�����Լӿ���������        */
    uint8 *code_relocate_ram = __section_begin("CodeRelocateRam");
    uint8 *code_relocate = __section_begin("CodeRelocate");
    uint8 *code_relocate_end = __section_end("CodeRelocate");

    /* ��ROM�︴�ƺ������뵽RAM�� */
    n = code_relocate_end - code_relocate;
    while (n--)
        *code_relocate_ram++ = *code_relocate++;
}

/*!
 *  @brief      ʱ�ӷ�Ƶ���ú���
 *  @param      outdiv1    �ں˷�Ƶϵ����       core    clk = MCG / (outdiv1 +1)
 *  @param      outdiv2    bus��Ƶϵ����        bus     clk = MCG / (outdiv2 +1)
 *  @param      outdiv3    flexbus��Ƶϵ����    flexbus clk = MCG / (outdiv3 +1)
 *  @param      outdiv4    flash��Ƶϵ����      flash   clk = MCG / (outdiv4 +1)
 *  @since      v1.0
 *  @author     ��˼������˾
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
    //�ϵ縴λ�󣬵�Ƭ�����Զ����� FEI ģʽ��ʹ�� �ڲ��ο�ʱ��

    //FEI -> FBE
    MCG_C2 &= ~MCG_C2_LP_MASK;
    MCG_C2 |= MCG_C2_RANGE(1);
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(7); 
    while (MCG_S & MCG_S_IREFST_MASK) {};                                //�ȴ�FLL�ο�ʱ�� Ϊ �ⲿ�ο�ʱ�ӣ�S[IREFST]=0����ʾʹ���ⲿ�ο�ʱ�ӣ���
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {}; //�ȴ�ѡ���ⲿ�ο�ʱ��
    //�����Ѿ������� FBEģʽ

    //FBE -> PBE
    set_sys_dividers(0, 1, 9, 3); //����ϵͳ��Ƶ����ѡ��
    MCG_C5 = MCG_C5_PRDIV(4);                       //��Ƶ�� EXTAL_IN_MHz/( PRDIV+1)
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(20);    //��Ƶ�� EXTAL_IN_MHz/( PRDIV+1)  * (VDIV+24)
    while (!(MCG_S & MCG_S_PLLST_MASK)) {};         //�ȴ�ʱ��Դѡ��PLL
    while (!(MCG_S & MCG_S_LOCK_MASK)) {};          //�ȴ� PLL���ˣ����໷��
    // �����Ѿ������� PBE ģʽ
    
    // PBE -> PEE
    MCG_C1 &= ~MCG_C1_CLKS_MASK;
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};//�ȴ�ѡ�����PLL
    // �����Ѿ������� PEE ģʽ
}

void start(void)
{
    wdogDisable();
    commonStartup();    // �����ж������� �� ��Ҫ�����ݵ� RAM��
    //pllInit();          // ϵͳ��ʼ��������ϵͳƵ�ʣ���ʼ��printf�˿�
    main();             // ִ���û�������
    while(1);          // ��ѭ��
}

