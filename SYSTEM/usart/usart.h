#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
// USART HEADER FILE
//********************************************************************************

#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define	CMD_BUF_LEN				4

extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
extern char Sci_cmd_buf[CMD_BUF_LEN];

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
char	crc_calc(char *buf, int buf_len);
#endif


