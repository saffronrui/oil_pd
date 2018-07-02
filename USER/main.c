//****************************************************************//
// 油动多旋翼无人机配电盒控制程序
// 处理器 stm32f103vet
// github: https://github.com/saffronrui/oil_pd
// 使用 github 时需要注意： 系统文件夹中忽略了 OBJ 文件夹，所以 clone 项目到本地之后需要手动建立空的 OBJ 文件夹
// 2018.7.1 20:00 @liuruirui
//***************************************************************//

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "spi.h"
#include "adc.h"
#include "iwdg.h"

u8 Tx_data[34];
u8 Sci_cmd[3];
u8 Sci_cmd_sta;		// 接收命令标志，= 0表示没有接收到命令帧头, = 1表示命令接收到帧头EB
									// = 2表示接收到二级帧头 CD， = 3表示命令接收到实际命令字符
									// = E0 表示全关； = EF 表示全开； 

u16	time_cnt;
float	STM_ADC_DATA_f[16];
float	STM_ADC_P[16];
float STM_ADC_F[16];

void Sci_Cmd_function(void);
void Tx_data_function(void);

 int main(void)
 { 
	u16 adcx[16], addata;
	u8 i;
	 
	delay_init();	    	 //延时函数初始化	  

	LED_Init();		 //初始化与LED、RELAY连接端口初始化
 	GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);
	uart_init(115200);	 //串口初始化为9600
	Adc_Init();		  		 //ADC初始化	    	    
	
	SPI_ADC_Init();
	#if	MCP3208_N2_ENABLE
	SPI2_Init();					//初始化备用MCP3208
  #endif

	STM_ADC_P[0] 	= 0.001204172; STM_ADC_F[0]  = 0.001537505;						//电流采集CH1系数
	STM_ADC_P[1] 	= 0.001206592; STM_ADC_F[1]  = 0.003148714;						//电流采集CH2系数
	STM_ADC_P[2] 	= 0.001206067; STM_ADC_F[2]  = 0.002076778;						//电流采集CH3系数
	STM_ADC_P[3] 	= 0.001205098; STM_ADC_F[3]  = 0.001673464;						//电流采集CH4系数
	STM_ADC_P[4] 	= 0.001206067; STM_ADC_F[4]  = 0.000870711;						//电流采集CH5系数
	STM_ADC_P[5] 	= 0.001205622; STM_ADC_F[5]  = 0.001238956;						//电流采集CH6系数
	STM_ADC_P[6] 	= 0.001203609; STM_ADC_F[6]  = 0.001804812;						//电流采集CH7系数
	STM_ADC_P[7] 	= 0.001205058; STM_ADC_F[7]  = 0.004016861;						//电流采集CH8系数
	STM_ADC_P[8] 	= 0.001208981; STM_ADC_F[8]  = 0.003190616;						//电流采集CH9系数
	STM_ADC_P[9] 	= 0.001208008; STM_ADC_F[9]  = 0.002685352;						//电流采集CH10系数
	STM_ADC_P[10] = .0; STM_ADC_F[10] = .0;
	STM_ADC_P[11] = .0; STM_ADC_F[11] = .0;
	STM_ADC_P[12] = .0; STM_ADC_F[12] = .0;
	STM_ADC_P[13] = .0; STM_ADC_F[13] = .0;
	STM_ADC_P[14] = .0; STM_ADC_F[14] = .0;
	STM_ADC_P[15] = .0; STM_ADC_F[15] = .0;

	 for(i = 0; i < 16; i++)
	{
		adcx[i] = 0;
		STM_ADC_DATA_f[i] = 0.0;
	}

	addata = 0;
	time_cnt = 0;
	 
	IWDG_Init(4,625);    //与分频数为64,重载值为625,溢出时间为1s
	
	 while(1)
	{
		Sci_Cmd_function();				//处理串口命令
		
		adcx[0]=Get_Adc_Average(ADC_Channel_0,10);					//电流采样通道10
		adcx[1]=Get_Adc_Average(ADC_Channel_1,10);					//电流采样通道9
		adcx[2]=Get_Adc_Average(ADC_Channel_2,10);					//电压采样通道5	
		adcx[3]=Get_Adc_Average(ADC_Channel_3,10);					//电压采样通道6
		adcx[8]=Get_Adc_Average(ADC_Channel_8,10);					//电流采样通道7
		adcx[9]=Get_Adc_Average(ADC_Channel_9,10);					//电流采样通道6
		adcx[10]=Get_Adc_Average(ADC_Channel_10,10);				//电流采样通道4
		adcx[11]=Get_Adc_Average(ADC_Channel_11,10);				//电流采样通道1
		adcx[12]=Get_Adc_Average(ADC_Channel_12,10);				//电流采样通道2
		adcx[13]=Get_Adc_Average(ADC_Channel_13,10);			  //电流采样通道3
		adcx[14]=Get_Adc_Average(ADC_Channel_14,10);				//电流采样通道5
		adcx[15]=Get_Adc_Average(ADC_Channel_15,10);				//电流采样通道8
		
		STM_ADC_DATA_f[0] =  adcx[0]  * STM_ADC_P[9] + STM_ADC_F[9] ;					//电流采样通道10
		STM_ADC_DATA_f[1] =  adcx[1]  * STM_ADC_P[8] + STM_ADC_F[8];					//电流采样通道9
		STM_ADC_DATA_f[8] =  adcx[8]  * STM_ADC_P[6] + STM_ADC_F[6];				//电流采样通道7
		STM_ADC_DATA_f[9] =  adcx[9]  * STM_ADC_P[5] + STM_ADC_F[5];				//电流采样通道6
		STM_ADC_DATA_f[10] = adcx[10] * STM_ADC_P[3] + STM_ADC_F[3];				//电流采样通道4
		STM_ADC_DATA_f[11] = adcx[11] * STM_ADC_P[0] + STM_ADC_F[0] ;				//电流采样通道1
		STM_ADC_DATA_f[12] = adcx[12] * STM_ADC_P[1] + STM_ADC_F[1] ;				//电流采样通道2
		STM_ADC_DATA_f[13] = adcx[13] * STM_ADC_P[2] + STM_ADC_F[2];				//电流采样通道3
		STM_ADC_DATA_f[14] = adcx[14] * STM_ADC_P[4] + STM_ADC_F[4];				//电流采样通道5
		STM_ADC_DATA_f[15] = adcx[15] * STM_ADC_P[7] + STM_ADC_F[7];				//电流采样通道8		
		
		Tx_data_function();
	
		for( i = 0; i < 34; i++ )							//	输出数据
		{
						printf("%c", Tx_data[i]);
		}
		
		LED0=!LED0;		//翻转控制板LED
		delay_ms(10);
		IWDG_Feed();				//	喂狗
	}											    
}	
 



void Sci_Cmd_function(void)				//处理串口命令
{
		char CMD_Val;
		int i;
	
		if( Sci_cmd_buf[0] == 0xEB )
				if( Sci_cmd_buf[1] == 'E' )
							{
								if(crc_clac(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// 计算 crc 验证码时候需要去掉最后一个字节，所以长度为 CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//处理一次命令后清空接收缓存区，等待下一次命令
										Sci_cmd_buf[i] = 0x00;
							}
		 
		switch(CMD_Val)
		{
				case 0xE0: GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);   break;
				case 0xEF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);GPIO_ResetBits(GPIOB,GPIO_Pin_5);	break;
				case	0xA0: GPIO_SetBits(GPIOC,GPIO_Pin_14);	  break;
				case	0xAF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14);	break;
				case	0xB0:	GPIO_SetBits(GPIOC,GPIO_Pin_15);   	break;
				case	0xBF:	GPIO_ResetBits(GPIOC,GPIO_Pin_15); 	break;
				case	0xC0:	GPIO_SetBits(GPIOB,GPIO_Pin_5);	    break;
				case	0xCF:	GPIO_ResetBits(GPIOB,GPIO_Pin_5);   break;	
				default : break;
		}

}

void Tx_data_function(void)
{
		u16 temp;
		u8	i;
		char	CRC_Byte;
	
		Tx_data[0]  = 0xEB;
		Tx_data[1]  = 0xCD;
		Tx_data[2]  = 0x20;
		Tx_data[3]  = 'T';
		
	  Tx_data[4]  = STM_ADC_DATA_f[11];						// 电流1数据
		Tx_data[5]  = STM_ADC_DATA_f[12];						// 电流1数据
		Tx_data[6]  = STM_ADC_DATA_f[13];						// 电流1数据
		Tx_data[7]  = STM_ADC_DATA_f[10];						// 电流1数据
		Tx_data[8]  = STM_ADC_DATA_f[14];						// 电流1数据
		Tx_data[9]  = STM_ADC_DATA_f[9];						// 电流1数据
		Tx_data[10] = STM_ADC_DATA_f[8];						// 电流1数据
		Tx_data[11] = STM_ADC_DATA_f[15];						// 电流1数据
		Tx_data[12] = STM_ADC_DATA_f[1] * 10;				// 电流1数据
				
		temp = 0;
		temp = STM_ADC_DATA_f[2] * 10;
		Tx_data[21] = temp;
		Tx_data[22] = temp >> 8;

		temp = 0;
		temp = STM_ADC_DATA_f[3] * 10;
		Tx_data[23] = temp;
		Tx_data[24] = temp >> 8;
		
		CRC_Byte = 0x00;
		for( i = 0; i< 33; i++ )
		{
				CRC_Byte ^= Tx_data[i];
		}
		Tx_data[33] = CRC_Byte;
}



