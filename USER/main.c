//****************************************************************//
// 油动多旋翼无人机配电盒控制程序
// 处理器 stm32f103vet
// github: https://github.com/saffronrui/oil_pd
// 使用 github 时需要注意： 系统文件夹中忽略了 OBJ 文件夹，所以 clone 项目到本地之后需要手动建立空的 OBJ 文件夹
// 双通道频率采集功能，采集范围 800 ~ 9000 rpm
// 2018.7.1 20:00 @liuruirui
//***************************************************************//

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "spi.h"
#include "adc.h"
#include "iwdg.h"
#include "timer.h"
#include "24cxx.h" 
#include "myiic.h"
#include "dma.h"

#define	Is_Debug	0			// Is_Debug == 1时调试模式，输出调试信息

//********************************************************
//********************************************************
//***********  软件版本号管理，升级软件时务必更改 ********

const u8 Program_Version[]={"***** V1.0.0--2018.9.5--By_LRR *****"};
#define	version_addr	1024

//***********  软件版本号管理，升级软件时务必更改 ********
//********************************************************
//********************************************************

u8 Sci_cmd[3];
u8 Sci_cmd_sta;		// 接收命令标志，= 0表示没有接收到命令帧头, = 1表示命令接收到帧头EB
									// = 2表示接收到二级帧头 CD， = 3表示命令接收到实际命令字符
									// = E0 表示全关； = EF 表示全开； 

#define	ADC_CH_NUM	9		//ADC采样通道数定义

float	STM_ADC_DATA_f[ADC_CH_NUM];
float	STM_ADC_P[ADC_CH_NUM];
float STM_ADC_F[ADC_CH_NUM];

const u8 TEXT_Buffer[]={"liuruirui---at24c02"};

char	Fault_sta = 0x00;
 
void Sci_Cmd_function(void);
void Tx_data_function(void);
char Voltage_Current_Protection(void);

 int main(void)
 { 
	u16 adcx[ADC_CH_NUM];
	u8 i;
	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2	 
	
	delay_init();	    	 //延时函数初始化	  
	
	uart_init(19200);	 //串口初始化为19200
	
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)Tx_Buf,Tx_Len);//DMA1通道4,外设为串口1,存储器为Tx_buf,长度为 sizeof(tx_buf)
	
	LED_Init();		 //初始化与LED、RELAY连接端口初始化   
	
	AT24CXX_Init();			//IIC初始化 
//	AT24CXX_Write(version_addr,(u8*)Program_Version,sizeof(Program_Version));
	 
	TIM1_PWM_Init(899,9); 			  //不分频。PWM频率=72000/(8999+1)=8Khz, 产生测试pwm
																// 频率测量范围 800~9000
	TIM_SetCompare1(TIM1,70);
	
	TIM2_Config();			// TIM2 定时中断设置，250ms 中断
	TIM3_Counter_Config();		// TIM3计数器设置
	TIM4_Counter_Config();	// TIM4计数器设置
	GPIO_Counter_Config();		// 	捕获端口设置
	 
	Adc_Init();		  		 //ADC初始化, 初始化时将 ADC 放到最后
	 
	STM_ADC_P[0] 	= 0.001204172; STM_ADC_F[0]  = 0.001537505;						//电流采集CH1系数
	STM_ADC_P[1] 	= 0.001206592; STM_ADC_F[1]  = 0.003148714;						//电流采集CH2系数
	STM_ADC_P[2] 	= 0.001206067; STM_ADC_F[2]  = 0.002076778;						//电流采集CH3系数
	STM_ADC_P[3] 	= 0.001205098; STM_ADC_F[3]  = 0.001673464;						//电流采集CH4系数
	STM_ADC_P[4] 	= 0.001206067; STM_ADC_F[4]  = 0.000870711;						//电流采集CH5系数
	STM_ADC_P[5] 	= 0.001205622; STM_ADC_F[5]  = 0.001238956;						//电流采集CH6系数
	STM_ADC_P[6] 	= 0.001203609; STM_ADC_F[6]  = 0.001804812;						//电流采集CH7系数
	STM_ADC_P[7] 	= 0.001205058; STM_ADC_F[7]  = 0.004016861;						//电流采集CH8系数
	STM_ADC_P[8] 	= 0.001208981; STM_ADC_F[8]  = 0.003190616;						//电流采集CH9系数

	 for(i = 0; i < ADC_CH_NUM; i++)
	{
		adcx[i] = 0;
		STM_ADC_DATA_f[i] = 0.0;
	}
	 
	IWDG_Init(4,625);    //与分频数为64,重载值为625,溢出时间为1s
	
	 while(1)
	{
		Sci_Cmd_function();				//处理串口命令
		

		
		// ADC 采样 9个通道采样计算耗时约 2ms
		adcx[0]=Get_Adc_Average(ADC_Channel_7,10);					//电流采样									-->PA7
		adcx[1]=Get_Adc_Average(ADC_Channel_10,10);					//12电压采样								-->PC0
		adcx[2]=Get_Adc_Average(ADC_Channel_11,10);					//24电压采样								-->PC1
		adcx[3]=Get_Adc_Average(ADC_Channel_13,10);					//温度1采样									-->PC3
		adcx[4]=Get_Adc_Average(ADC_Channel_12,10);					//温度2采样									-->PC2
		adcx[5]=Get_Adc_Average(ADC_Channel_0,10);					//温度3采样									-->PA0
		adcx[6]=Get_Adc_Average(ADC_Channel_1,10);					//温度4采样									-->PA1
		adcx[7]=Get_Adc_Average(ADC_Channel_2,10);					//备用电压采样1  0-10V			-->PA2
		adcx[8]=Get_Adc_Average(ADC_Channel_3,10);					//备用电压采样2  0-10V			-->PA3

#if	Is_Debug	// DEBUG模式修改 Is_Debug 参数输出采集的原始值

		USART_DMACmd(USART1,USART_DMAReq_Tx,DISABLE); 											// 调试模式下禁 DMA 输出，即只输出调试数据
																																				// 不输出正常数据
		
		for( i = 3; i < 7; i++ ){
				printf("%c", adcx[i] >> 8);
				printf("%c", adcx[i]);
		}	

//				printf("%c", adcx[1] >> 8);
//				printf("%c", adcx[1]);
//				printf("%c", adcx[2] >> 8);
//				printf("%c", adcx[2]);
		
#endif	
		
		// ADC 计算拟合数据
//		STM_ADC_DATA_f[0] =  adcx[0]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//电流采样通道10
//		STM_ADC_DATA_f[1] =  adcx[1]  * STM_ADC_P[8] + STM_ADC_F[8];					//电流采样通道9
//		STM_ADC_DATA_f[2] =  adcx[2]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//电流采样通道10
//		STM_ADC_DATA_f[3] =  adcx[3]  * STM_ADC_P[8] + STM_ADC_F[8];					//电流采样通道9
//		STM_ADC_DATA_f[4] =  adcx[4]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//电流采样通道10
//		STM_ADC_DATA_f[5] =  adcx[5]  * STM_ADC_P[8] + STM_ADC_F[8];					//电流采样通道9	
//		STM_ADC_DATA_f[6] =  adcx[6]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//电流采样通道10
//		STM_ADC_DATA_f[7] =  adcx[7]  * STM_ADC_P[8] + STM_ADC_F[8];					//电流采样通道9	
//		STM_ADC_DATA_f[8] =  adcx[8]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//电流采样通道10
	
		STM_ADC_DATA_f[0] =  1.51 * adcx[0]  * 3.3 * 10 / 4096;					//电流
		STM_ADC_DATA_f[1] =  6.0 * adcx[1]  * 3.3 * 10 / 4096;					//12电压
		STM_ADC_DATA_f[2] =  adcx[2]  * 3.3 * 10 / 4096;					//电流采样通道10
		STM_ADC_DATA_f[3] =  adcx[3]  * 3.3 * 10 / 4096;					//电流采样通道9
		STM_ADC_DATA_f[4] =  adcx[4]  * 3.3 * 10 / 4096 ;					//电流采样通道10
		STM_ADC_DATA_f[5] =  adcx[5]  * 3.3 * 10 / 4096;					//电流采样通道9	
		STM_ADC_DATA_f[6] =  adcx[6]  * 3.3 * 10 / 4096 ;					//电流采样通道10
		STM_ADC_DATA_f[7] =  adcx[7]  * 3.3 * 10 / 4096;					//电流采样通道9	
		STM_ADC_DATA_f[8] =  adcx[8]  * 3.3 * 10 / 4096 ;					//电流采样通道10
	
		Fault_sta = Voltage_Current_Protection();			//设备保护并返回相应故障码
	
	
		Tx_data_function();	
		
		delay_ms(500);
		IWDG_Feed();				//	喂狗
	}											    
}	
 

char Voltage_Current_Protection(void)
{

		if( STM_ADC_DATA_f[0] > 20. ){			//过流保护
				//关闭所有mosfet通道
				return 0xBB;
		} 	
		
		if( (STM_ADC_DATA_f[1] > 26. ) || (STM_ADC_DATA_f[1] < 18.) ){			// 24V电源高压低压保护
				//关闭所有moefet通道
				return 0xCC;
		}
		
		if( (STM_ADC_DATA_f[2] > 14. ) || (STM_ADC_DATA_f[1] < 11.) ){			// 12V电源高压低压保护
				//关闭所有moefet通道
			return	0xEE;
		}				
		
		return 0x00;  // 能执行到这一步表示上面错误没有出现，否则函数已经返回
}

void Sci_Cmd_function(void)				//处理串口命令
{
		char CMD_Val;
		int i;
		u8 datatemp[40];
	
		if( Sci_cmd_buf[0] == 0xEB )
				if( Sci_cmd_buf[1] == 'E' )
							{
	//							if(crc_calc(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// 计算 crc 验证码时候需要去掉最后一个字节，所以长度为 CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//处理一次命令后清空接收缓存区，等待下一次命令
										Sci_cmd_buf[i] = 0x00;
							}
		 
		switch(CMD_Val)
		{
				case	0xE0: GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);   break;
				case	0xEF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);GPIO_ResetBits(GPIOB,GPIO_Pin_5);	break;
				case	0xA0: GPIO_SetBits(GPIOC,GPIO_Pin_14);	  break;
				case	0xAF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14);	break;
				case	0xB0:	GPIO_SetBits(GPIOC,GPIO_Pin_15);   	break;
				case	0xBF:	GPIO_ResetBits(GPIOC,GPIO_Pin_15); 	break;
				case	0xC0:	GPIO_SetBits(GPIOB,GPIO_Pin_5);	    break;
				case	0xCF:	GPIO_ResetBits(GPIOB,GPIO_Pin_5);   break;	
			
				case	'V':	AT24CXX_Read(version_addr,datatemp,20);	printf("%s\n", datatemp);		break;				// 软件版本号码查询
				default : break;
		}

}

void Tx_data_function(void)
{
#if !Is_Debug 																		// 是否为调试模式
		
		Tx_Buf[0]  = 0xEE;														// 帧头
		Tx_Buf[1]  = 'P';															// 标志位
		Tx_Buf[2]  = Tx_Len;													// 数组长度
		Tx_Buf[3]  = STM_ADC_DATA_f[0];											
		Tx_Buf[4]  = STM_ADC_DATA_f[1];
		Tx_Buf[5]  = STM_ADC_DATA_f[2];
		Tx_Buf[6]  = STM_ADC_DATA_f[3];
		Tx_Buf[7]  = STM_ADC_DATA_f[4];
		Tx_Buf[8]  = STM_ADC_DATA_f[5];
		Tx_Buf[9]  = STM_ADC_DATA_f[6];
		Tx_Buf[10] = STM_ADC_DATA_f[7];
		Tx_Buf[11] = STM_ADC_DATA_f[8];
		
		if(Timer3_freq1 < 50)													// 转速小于 50 rmp 不显示转速								
				Timer3_freq1 = 0;
		if(Timer4_freq2 < 50)
				Timer4_freq2 = 0;
		
		Tx_Buf[12] = Timer3_freq1 >> 8;
		Tx_Buf[13] = Timer3_freq1;
		Tx_Buf[14] = Timer4_freq2 >> 8;
		Tx_Buf[15] = Timer4_freq2;	
									
		Tx_Buf[19] = crc_calc(Tx_Buf, Tx_Len);							//	CRC验证码计算
	
#endif
}





