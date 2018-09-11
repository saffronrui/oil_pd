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
#include "pt1000.h"

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

typedef union	def_float
{
		char	fchar[4];
		float ff;
};


void Sci_Cmd_function(void);
void Tx_data_function(void);
char Voltage_Current_Protection(void);
//void Push_EE_float(u16 addr, union	def_float	my_float);
////void Pop_EE_float(u16 addr, union def_float	*my_float	);

//void Push_EE_float(u16 addr, union	def_float	my_float)
//{
//		AT24CXX_WriteOneByte(addr++, my_float.fchar[0]);
//		AT24CXX_WriteOneByte(addr++, my_float.fchar[1]);
//		AT24CXX_WriteOneByte(addr++, my_float.fchar[2]);
//		AT24CXX_WriteOneByte(addr++, my_float.fchar[3]);
//}

 int main(void)
 { 
	u16 adcx[ADC_CH_NUM];
	u8 i;
	
	 union def_float	float_type;
	 union def_float	f1_type;
	 
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2	 
	
	delay_init();	    	 //延时函数初始化	  
	
	uart_init(19200);	 //串口初始化为19200
	
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)Tx_Buf,Tx_Len);//DMA1通道4,外设为串口1,存储器为Tx_buf,长度为 sizeof(tx_buf)
	
	LED_Init();		 //初始化与LED、RELAY连接端口初始化   
	
	AT24CXX_Init();			//IIC初始化 
//	AT24CXX_Write(version_addr,(u8*)Program_Version,sizeof(Program_Version));

//-------------------------------------------------------------------------------//	 
// timer1 输出pwm用来测试频率采集，需要时去掉注释即可
// 需要注意， PA8 输出测试 pwm，此端口与 mosfet 控制端口复用

	TIM1_PWM_Init(899,99); 			  //不分频。PWM频率=72000/(8999+1)=8Khz, 产生测试pwm
																// 频率测量范围 800~9000
	TIM_SetCompare1(TIM1,70);
//-------------------------------------------------------------------------------//	
	
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
	
//	float_type.ff = 3.1415926;
//	AT24CXX_WriteOneByte(0, float_type.fchar[0]);
//	AT24CXX_WriteOneByte(1, float_type.fchar[1]);
//	AT24CXX_WriteOneByte(2, float_type.fchar[2]);
//	AT24CXX_WriteOneByte(3, float_type.fchar[3]);	
	

	 
	while(1)
	{
		Sci_Cmd_function();				//处理串口命令
		
//		f1_type.fchar[0] = AT24CXX_ReadOneByte(0);
//		f1_type.fchar[1] = AT24CXX_ReadOneByte(1);
//		f1_type.fchar[2] = AT24CXX_ReadOneByte(2);
//		f1_type.fchar[3] = AT24CXX_ReadOneByte(3);
//		printf("%f", f1_type.ff);
		
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
		
//		for( i = 3; i < 7; i++ ){
//				printf("%c", adcx[1] >> 8);
//				printf("%c", adcx[1]);
//		}	

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
	
		STM_ADC_DATA_f[0] =  1.51 * adcx[0]  * 3.3 * 10 / 4096;		//电流采样									-->PA7
		STM_ADC_DATA_f[1] =  adcx[1]*0.048197;										//12电压采样								-->PC0
		STM_ADC_DATA_f[2] =  adcx[2]*0.2789 - 0.7849 ;						//24电压采样								-->PC1

//		STM_ADC_DATA_f[3] =  adcx[3];														//温度1采样									-->PC3
		STM_ADC_DATA_f[3] = Get_Temputure(adcx[3], 0);															//温度1采样		
		STM_ADC_DATA_f[4] = Get_Temputure(adcx[4], 1);															//温度2采样									-->PC2
		STM_ADC_DATA_f[5] = Get_Temputure(adcx[5], 2);															//温度3采样									-->PA0
		STM_ADC_DATA_f[6] = Get_Temputure(adcx[6], 3);															//温度4采样									-->PA1
		
		STM_ADC_DATA_f[7] =  adcx[7];					//备用电压采样1  0-10V			-->PA2
		STM_ADC_DATA_f[8] =  adcx[8];					//备用电压采样2  0-10V			-->PA3
	
		Fault_sta = Voltage_Current_Protection();			//设备保护并返回相应故障码
	
	
		Tx_data_function();	
		
		delay_ms(200);
		IWDG_Feed();				//	喂狗
	}											    
}	
 

char Voltage_Current_Protection(void)
{

		if( STM_ADC_DATA_f[0] > 20. ){			//过流保护
				//关闭所有mosfet通道
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
				return 0xBB;
		} 	
		
		if( (STM_ADC_DATA_f[2] > 260 ) || (STM_ADC_DATA_f[2] < 180) ){			// 24V电源高压低压保护
				 //关闭所有moefet通道
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
				return 0xCC;
		}
		
		if( (STM_ADC_DATA_f[1] > 130 ) || (STM_ADC_DATA_f[1] < 105) ){			// 12V电源高压低压保护
				 //关闭所有moefet通道
				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
			
			return	0xAF;
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
								LED1 = !LED1;
	//							if(crc_calc(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// 计算 crc 验证码时候需要去掉最后一个字节，所以长度为 CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//处理一次命令后清空接收缓存区，等待下一次命令
										Sci_cmd_buf[i] = 0x00;
							}
		 
		switch(CMD_Val)
		{	
				case	0xE0: GPIO_SetBits(GPIOB,GPIO_Pin_4);      	break;				// 停车开关 1 动作
				case	0xEA:	GPIO_ResetBits(GPIOB,GPIO_Pin_4);			break;				// 停车开关 1 复位
				
				case	0xF0: GPIO_SetBits(GPIOB,GPIO_Pin_5);     	break;				// 停车开关 2 动作
				case	0xFA:	GPIO_ResetBits(GPIOB,GPIO_Pin_5);			break;				// 停车开关 2 复位
				
				case	0xA0: GPIO_SetBits(GPIOA,GPIO_Pin_11);	  	break;				// mosfet1 turn-on
				case	0xAA:	GPIO_ResetBits(GPIOA,GPIO_Pin_11);		break;				// mosfet1 turn-off
				
				case	0xB0:	GPIO_SetBits(GPIOA,GPIO_Pin_8);   		break;				// mosfet2 turn-on
				case	0xBA:	GPIO_ResetBits(GPIOA,GPIO_Pin_8); 		break;				// mosfet2 turn-off
				
				case	0xC0:	GPIO_SetBits(GPIOE,GPIO_Pin_2);	    	break;				// mosfet3 turn-on
				case	0xCA:	GPIO_ResetBits(GPIOE,GPIO_Pin_2);   	break;				// mosfet3 turn-off
				
				case	0xD0:	GPIO_SetBits(GPIOE,GPIO_Pin_3);		    break;				// mosfet4 turn-on
				case	0xDA:	GPIO_ResetBits(GPIOE,GPIO_Pin_3);   	break;				// mosfet4 turn-off
							
				case	0x80:	GPIO_SetBits(GPIOE,GPIO_Pin_1);	    	break;				// mosfet5 turn-on				//暂时没有焊接
				case	0x8A:	GPIO_ResetBits(GPIOE,GPIO_Pin_1);   	break;				// mosfet5 turn-off
				
				case	0x90:	GPIO_SetBits(GPIOE,GPIO_Pin_4);		    break;				// mosfet6 turn-on				//暂时没有焊接
				case	0x9A:	GPIO_ResetBits(GPIOE,GPIO_Pin_4);   	break;				// mosfet6 turn-off
			
				case	'V':	AT24CXX_Read(version_addr,datatemp,20);	printf("%s\n", datatemp);		break;				// 软件版本号码查询
				default : break;
		}

}

void Tx_data_function(void)
{
		u8	frame_cnt;
		u16 freq1, freq2;
#if !Is_Debug 																		// 是否为调试模式
		
		frame_cnt = 0;
	
		Tx_Buf[frame_cnt++]  = 0xEE;														// 帧头
		Tx_Buf[frame_cnt++]  = 'P';															// 标志位
		Tx_Buf[frame_cnt++]  = Tx_Len;													// 数组长度
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[0];								// 电流采集							
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[1];								// 12V 电压采集
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[2] >> 8;						// 24V 电压采集高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[2];								// 24V 电压采集低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[3] >> 8;						// 温度采集1高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[3];								// 温度采集1低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[4] >> 8;						// 温度采集2高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[4];								// 温度采集2低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[5] >> 8;						// 温度采集3高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[5];								// 温度采集3低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[6] >> 8;						// 温度采集4高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[6];								// 温度采集4低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[7] >> 8;						// 备用AD通道1高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[7];								// 备用AD通道1低字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[8] >> 8;						// 备用AD通道2高字节
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[8];								// 备用AD通道2低字节
		
		freq1 = Timer3_freq1 * 4.0020;		//	转速系数调整，标准系数为 250ms 一次中断， *4，最好避免在中断函数内做运算
		freq2 = Timer4_freq2 * 4.0020;
		
				
		if(freq1 < 50)													// 转速小于 50 rmp 不显示转速								
				freq1 = 0;
		if(freq2 < 50)
				freq2 = 0;
		
		Tx_Buf[frame_cnt++] = freq1 >> 8;
		Tx_Buf[frame_cnt++] = freq1;
		Tx_Buf[frame_cnt++] = freq2 >> 8;
		Tx_Buf[frame_cnt++] = freq2;	
									
		Tx_Buf[Tx_Len-3] = Fault_sta;															//	故障状态显示, 0XAF 表示没有故障
		Tx_Buf[Tx_Len-1] = crc_calc(Tx_Buf, Tx_Len);							//	CRC验证码计算
	
#endif
}





