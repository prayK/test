#ifndef __MISC_H__
#define __MISC_H__

#include "common.h"

void write_vtor (int vtor);
void set_vector_handler(VECTORn_t vector , void pfunc_handler(void));      //设置中断函数到中断向量表里

//兼容旧代码
#define enable_irq(irq)                 NVIC_EnableIRQ(irq)         //使能IRQ
#define disable_irq(irq)                NVIC_DisableIRQ(irq)        //禁止IRQ
#define set_irq_priority(irq,pri0)      NVIC_SetPriority(irq,pri0)  //设置优先级
#define EnableInterrupts                asm(" CPSIE i");              //使能全部中断
#define DisableInterrupts               asm(" CPSID i");             //禁止全部中断

#endif

