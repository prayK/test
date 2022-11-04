#include "MK60DZ10.h"

static void ledGpioInit(void)
{
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK << 0U);     //开启PORTA端口         
    PORT_PCR_REG(PORTA_BASE_PTR, 17U) = 0X01U << PORT_PCR_MUX_SHIFT;    //将PORTA端口第17号引脚复用为GPIO
    GPIO_PDDR_REG(PTA_BASE_PTR) |= 0X01U << 17U;    //设置PORTA 17设置为输出
}

void main(void)
{
    ledGpioInit();
    while(1)
    {
        GPIO_PDOR_REG(PTA_BASE_PTR) &= ~0X01U << 17U;
    }
}
