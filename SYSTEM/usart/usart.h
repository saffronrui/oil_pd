#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
// USART HEADER FILE
//********************************************************************************

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
#define	CMD_BUF_LEN				4

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern char Sci_cmd_buf[CMD_BUF_LEN];

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
char	crc_calc(char *buf, int buf_len);
#endif


