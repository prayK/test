#include "common.h"
#include "assert.h"

/*!
 *  @brief      �����ж��������ַ
 *  @param      vtor    �µ��ж��������ַ
 *  @since      v5.0
 *  @author     ��˼������˾
 *  Sample usage:       write_vtor ((uint32)__VECTOR_RAM);  //�µ��ж�������ַ
 */
void write_vtor (int vtor)
{
    assert(vtor % 0x200 == 0);   //Vector Table base offset field. This value must be a multiple of 0x200.

    /* Write the VTOR with the new value */
    SCB_VTOR = vtor;
}

/*!
 *  @brief      �����ж�����������жϷ�����
 *  @since      v5.0
 *  @warning    ֻ���ж�������λ��icfָ����RAM����ʱ���˺�������Ч
 *  Sample usage:       set_vector_handler(UART3_RX_TX_VECTORn , uart3_handler);    //�� uart3_handler ������ӵ��ж�������
 */
void set_vector_handler(VECTORn_t vector , void pfunc_handler(void))
{
    extern uint32 __VECTOR_RAM[];

    assert(SCB_VTOR == (uint32)__VECTOR_RAM);  //���ԣ�����ж��������Ƿ��� RAM ��

    __VECTOR_RAM[vector] = (uint32)pfunc_handler;
}