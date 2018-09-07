#include "sys.h"
#include "usart.h"	  
#include "led.h"

extern u8 Sci_cmd[3];
extern u8 Sci_cmd_sta;		
char	Sci_cmd_buf[CMD_BUF_LEN] = {0};

char		Tx_Buf[Tx_Len] = {0};
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
	
//   USART_SendData(USART1, (u8) ch);

//  
//   while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET))
//   {
//   }

//   return ch;
}
#endif 

 
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
#if	RS232_ENABLE
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2，GPIOD时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD);

	//USART2_TX   GPIOD.5
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //PD.5
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD.5
 
	//USART2_RX	  GPIOD.6初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//PD6
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD.6  

  //Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;//抢占优先级4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART2 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口2
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART2, ENABLE);                    //使能串口2 
#endif
	
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure);			 //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //开启串口接受中断
  USART_Cmd(USART1, ENABLE);                     //使能串口1 

}


void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
		u8 Res;
		static int cmd_count = 0;
		//  u8 Sci_cmd_sta;					// 接收命令标志，= 0表示没有接收到命令帧头, = 1表示命令接收到帧头EB
																		// = 2表示接收到二级帧头 CD， = 3表示命令接收到实际命令字符
																		// = E0 表示全关； = EF 表示全开； = A0 表示继电器A关闭，AF表示继电器A导通
																		// = B0 表示继电器B关闭，BF表示继电器B导通
																		// = C0 表示继电器C关闭，CF表示继电器C导通
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
			Res =USART_ReceiveData(USART1);						//读取接收到的数据
			
			if( (Res == 0xEB) || (cmd_count >= CMD_BUF_LEN))					// 判断帧头与接收数组溢出
					cmd_count = 0;
			Sci_cmd_buf[cmd_count++] = Res;
			
     } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}

//*******************************
// CRC计算函数，从 0 --> CMD_BUF_LEN - 2
// 输入： 数组指针， 数组长度
// 返回值： char CRC 计算值
// 2018.7.1 @ saffronui
//*******************************

char	crc_calc(char *buf, int buf_len)			//CRC 计算函数
{
		int i;
		char CRC_Byte;
		
		CRC_Byte = buf[0];
		
		for( i = 1; i < buf_len -1 ; i++ ){
				CRC_Byte ^= buf[i];
		}
		
		return	CRC_Byte;
}

#endif	



