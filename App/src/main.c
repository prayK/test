#include "MK60DZ10.h"

static void ledGpioInit(void)
{
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK << 0U);     //����PORTA�˿�         
    PORT_PCR_REG(PORTA_BASE_PTR, 17U) = 0X01U << PORT_PCR_MUX_SHIFT;    //��PORTA�˿ڵ�17�����Ÿ���ΪGPIO
    GPIO_PDDR_REG(PTA_BASE_PTR) |= 0X01U << 17U;    //����PORTA 17����Ϊ���
}

void main(void)
{
    ledGpioInit();
    while(1)
    {
        GPIO_PDOR_REG(PTA_BASE_PTR) &= ~0X01U << 17U;
    }
}
