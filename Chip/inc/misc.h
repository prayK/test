#ifndef __MISC_H__
#define __MISC_H__

#include "common.h"

void write_vtor (int vtor);
void set_vector_handler(VECTORn_t vector , void pfunc_handler(void));      //�����жϺ������ж���������

//���ݾɴ���
#define enable_irq(irq)                 NVIC_EnableIRQ(irq)         //ʹ��IRQ
#define disable_irq(irq)                NVIC_DisableIRQ(irq)        //��ֹIRQ
#define set_irq_priority(irq,pri0)      NVIC_SetPriority(irq,pri0)  //�������ȼ�
#define EnableInterrupts                asm(" CPSIE i");              //ʹ��ȫ���ж�
#define DisableInterrupts               asm(" CPSID i");             //��ֹȫ���ж�

#endif

