#include "MK60DZ10.h"

static void ledGpioInit(void)
{
    SIM_BASE_PTR->SCGC5 |= (SIM_SCGC5_PORTA_MASK << 0U);      //开启PORTA端口         
    PORTA_BASE_PTR->PCR[17U] = 0X01U << PORT_PCR_MUX_SHIFT;   //将PORTA端口第17号引脚复用为GPIO
    PTA_BASE_PTR->PDDR |= (0X01U << 17U);                     //设置PORTA 17设置为输出
    PTA_BASE_PTR->PDOR &= ~(0X01U << 17U);                    //清零输出低电平
}

void main(void)
{
    ledGpioInit();
    while(1);
}
