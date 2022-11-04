#include "misc.h"
#include "common.h"
#include "system_MK60DZ10.h"

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

void start(void)
{
    wdogDisable();
    commonStartup();    // �����ж������� �� ��Ҫ�����ݵ� RAM��
    main();             // ִ���û�������
    while(1);          // ��ѭ��
}

