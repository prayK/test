#include "MK60DZ10.h"

static void ledGpioInit(void)
{
    SIM_BASE_PTR->SCGC5 |= (SIM_SCGC5_PORTA_MASK << 0U);      //����PORTA�˿�         
    PORTA_BASE_PTR->PCR[17U] = 0X01U << PORT_PCR_MUX_SHIFT;   //��PORTA�˿ڵ�17�����Ÿ���ΪGPIO
    PTA_BASE_PTR->PDDR |= (0X01U << 17U);                     //����PORTA 17����Ϊ���
    PTA_BASE_PTR->PDOR &= ~(0X01U << 17U);                    //��������͵�ƽ
}

void main(void)
{
    ledGpioInit();
    while(1);
}
