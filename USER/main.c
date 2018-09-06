//****************************************************************//
// �Ͷ����������˻����п��Ƴ���
// ������ stm32f103vet
// github: https://github.com/saffronrui/oil_pd
// ʹ�� github ʱ��Ҫע�⣺ ϵͳ�ļ����к����� OBJ �ļ��У����� clone ��Ŀ������֮����Ҫ�ֶ������յ� OBJ �ļ���
// ˫ͨ��Ƶ�ʲɼ����ܣ��ɼ���Χ 800 ~ 9000 rpm
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

#define	Is_Debug	0			// Is_Debug == 1ʱ����ģʽ�����������Ϣ

//********************************************************
//********************************************************
//***********  ����汾�Ź����������ʱ��ظ��� ********

const u8 Program_Version[]={"***** V1.0.0--2018.9.5--By_LRR *****"};
#define	version_addr	1024

//***********  ����汾�Ź����������ʱ��ظ��� ********
//********************************************************
//********************************************************

u8 Sci_cmd[3];
u8 Sci_cmd_sta;		// ���������־��= 0��ʾû�н��յ�����֡ͷ, = 1��ʾ������յ�֡ͷEB
									// = 2��ʾ���յ�����֡ͷ CD�� = 3��ʾ������յ�ʵ�������ַ�
									// = E0 ��ʾȫ�أ� = EF ��ʾȫ���� 

#define	ADC_CH_NUM	9		//ADC����ͨ��������

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
	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2	 
	
	delay_init();	    	 //��ʱ������ʼ��	  
	
	uart_init(19200);	 //���ڳ�ʼ��Ϊ19200
	
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)Tx_Buf,Tx_Len);//DMA1ͨ��4,����Ϊ����1,�洢��ΪTx_buf,����Ϊ sizeof(tx_buf)
	
	LED_Init();		 //��ʼ����LED��RELAY���Ӷ˿ڳ�ʼ��   
	
	AT24CXX_Init();			//IIC��ʼ�� 
//	AT24CXX_Write(version_addr,(u8*)Program_Version,sizeof(Program_Version));
	 
	TIM1_PWM_Init(899,9); 			  //����Ƶ��PWMƵ��=72000/(8999+1)=8Khz, ��������pwm
																// Ƶ�ʲ�����Χ 800~9000
	TIM_SetCompare1(TIM1,70);
	
	TIM2_Config();			// TIM2 ��ʱ�ж����ã�250ms �ж�
	TIM3_Counter_Config();		// TIM3����������
	TIM4_Counter_Config();	// TIM4����������
	GPIO_Counter_Config();		// 	����˿�����
	 
	Adc_Init();		  		 //ADC��ʼ��, ��ʼ��ʱ�� ADC �ŵ����
	 
	STM_ADC_P[0] 	= 0.001204172; STM_ADC_F[0]  = 0.001537505;						//�����ɼ�CH1ϵ��
	STM_ADC_P[1] 	= 0.001206592; STM_ADC_F[1]  = 0.003148714;						//�����ɼ�CH2ϵ��
	STM_ADC_P[2] 	= 0.001206067; STM_ADC_F[2]  = 0.002076778;						//�����ɼ�CH3ϵ��
	STM_ADC_P[3] 	= 0.001205098; STM_ADC_F[3]  = 0.001673464;						//�����ɼ�CH4ϵ��
	STM_ADC_P[4] 	= 0.001206067; STM_ADC_F[4]  = 0.000870711;						//�����ɼ�CH5ϵ��
	STM_ADC_P[5] 	= 0.001205622; STM_ADC_F[5]  = 0.001238956;						//�����ɼ�CH6ϵ��
	STM_ADC_P[6] 	= 0.001203609; STM_ADC_F[6]  = 0.001804812;						//�����ɼ�CH7ϵ��
	STM_ADC_P[7] 	= 0.001205058; STM_ADC_F[7]  = 0.004016861;						//�����ɼ�CH8ϵ��
	STM_ADC_P[8] 	= 0.001208981; STM_ADC_F[8]  = 0.003190616;						//�����ɼ�CH9ϵ��

	 for(i = 0; i < ADC_CH_NUM; i++)
	{
		adcx[i] = 0;
		STM_ADC_DATA_f[i] = 0.0;
	}
	 
	IWDG_Init(4,625);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s
	
	 while(1)
	{
		Sci_Cmd_function();				//����������
		

		
		// ADC ���� 9��ͨ�����������ʱԼ 2ms
		adcx[0]=Get_Adc_Average(ADC_Channel_7,10);					//��������									-->PA7
		adcx[1]=Get_Adc_Average(ADC_Channel_10,10);					//12��ѹ����								-->PC0
		adcx[2]=Get_Adc_Average(ADC_Channel_11,10);					//24��ѹ����								-->PC1
		adcx[3]=Get_Adc_Average(ADC_Channel_13,10);					//�¶�1����									-->PC3
		adcx[4]=Get_Adc_Average(ADC_Channel_12,10);					//�¶�2����									-->PC2
		adcx[5]=Get_Adc_Average(ADC_Channel_0,10);					//�¶�3����									-->PA0
		adcx[6]=Get_Adc_Average(ADC_Channel_1,10);					//�¶�4����									-->PA1
		adcx[7]=Get_Adc_Average(ADC_Channel_2,10);					//���õ�ѹ����1  0-10V			-->PA2
		adcx[8]=Get_Adc_Average(ADC_Channel_3,10);					//���õ�ѹ����2  0-10V			-->PA3

#if	Is_Debug	// DEBUGģʽ�޸� Is_Debug ��������ɼ���ԭʼֵ

		USART_DMACmd(USART1,USART_DMAReq_Tx,DISABLE); 											// ����ģʽ�½� DMA �������ֻ�����������
																																				// �������������
		
		for( i = 3; i < 7; i++ ){
				printf("%c", adcx[i] >> 8);
				printf("%c", adcx[i]);
		}	

//				printf("%c", adcx[1] >> 8);
//				printf("%c", adcx[1]);
//				printf("%c", adcx[2] >> 8);
//				printf("%c", adcx[2]);
		
#endif	
		
		// ADC �����������
//		STM_ADC_DATA_f[0] =  adcx[0]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
//		STM_ADC_DATA_f[1] =  adcx[1]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9
//		STM_ADC_DATA_f[2] =  adcx[2]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
//		STM_ADC_DATA_f[3] =  adcx[3]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9
//		STM_ADC_DATA_f[4] =  adcx[4]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
//		STM_ADC_DATA_f[5] =  adcx[5]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9	
//		STM_ADC_DATA_f[6] =  adcx[6]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
//		STM_ADC_DATA_f[7] =  adcx[7]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9	
//		STM_ADC_DATA_f[8] =  adcx[8]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
	
		STM_ADC_DATA_f[0] =  1.51 * adcx[0]  * 3.3 * 10 / 4096;					//����
		STM_ADC_DATA_f[1] =  6.0 * adcx[1]  * 3.3 * 10 / 4096;					//12��ѹ
		STM_ADC_DATA_f[2] =  adcx[2]  * 3.3 * 10 / 4096;					//��������ͨ��10
		STM_ADC_DATA_f[3] =  adcx[3]  * 3.3 * 10 / 4096;					//��������ͨ��9
		STM_ADC_DATA_f[4] =  adcx[4]  * 3.3 * 10 / 4096 ;					//��������ͨ��10
		STM_ADC_DATA_f[5] =  adcx[5]  * 3.3 * 10 / 4096;					//��������ͨ��9	
		STM_ADC_DATA_f[6] =  adcx[6]  * 3.3 * 10 / 4096 ;					//��������ͨ��10
		STM_ADC_DATA_f[7] =  adcx[7]  * 3.3 * 10 / 4096;					//��������ͨ��9	
		STM_ADC_DATA_f[8] =  adcx[8]  * 3.3 * 10 / 4096 ;					//��������ͨ��10
	
		Fault_sta = Voltage_Current_Protection();			//�豸������������Ӧ������
	
	
		Tx_data_function();	
		
		delay_ms(500);
		IWDG_Feed();				//	ι��
	}											    
}	
 

char Voltage_Current_Protection(void)
{

		if( STM_ADC_DATA_f[0] > 20. ){			//��������
				//�ر�����mosfetͨ��
				return 0xBB;
		} 	
		
		if( (STM_ADC_DATA_f[1] > 26. ) || (STM_ADC_DATA_f[1] < 18.) ){			// 24V��Դ��ѹ��ѹ����
				//�ر�����moefetͨ��
				return 0xCC;
		}
		
		if( (STM_ADC_DATA_f[2] > 14. ) || (STM_ADC_DATA_f[1] < 11.) ){			// 12V��Դ��ѹ��ѹ����
				//�ر�����moefetͨ��
			return	0xEE;
		}				
		
		return 0x00;  // ��ִ�е���һ����ʾ�������û�г��֣��������Ѿ�����
}

void Sci_Cmd_function(void)				//����������
{
		char CMD_Val;
		int i;
		u8 datatemp[40];
	
		if( Sci_cmd_buf[0] == 0xEB )
				if( Sci_cmd_buf[1] == 'E' )
							{
	//							if(crc_calc(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// ���� crc ��֤��ʱ����Ҫȥ�����һ���ֽڣ����Գ���Ϊ CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//����һ���������ս��ջ��������ȴ���һ������
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
			
				case	'V':	AT24CXX_Read(version_addr,datatemp,20);	printf("%s\n", datatemp);		break;				// ����汾�����ѯ
				default : break;
		}

}

void Tx_data_function(void)
{
#if !Is_Debug 																		// �Ƿ�Ϊ����ģʽ
		
		Tx_Buf[0]  = 0xEE;														// ֡ͷ
		Tx_Buf[1]  = 'P';															// ��־λ
		Tx_Buf[2]  = Tx_Len;													// ���鳤��
		Tx_Buf[3]  = STM_ADC_DATA_f[0];											
		Tx_Buf[4]  = STM_ADC_DATA_f[1];
		Tx_Buf[5]  = STM_ADC_DATA_f[2];
		Tx_Buf[6]  = STM_ADC_DATA_f[3];
		Tx_Buf[7]  = STM_ADC_DATA_f[4];
		Tx_Buf[8]  = STM_ADC_DATA_f[5];
		Tx_Buf[9]  = STM_ADC_DATA_f[6];
		Tx_Buf[10] = STM_ADC_DATA_f[7];
		Tx_Buf[11] = STM_ADC_DATA_f[8];
		
		if(Timer3_freq1 < 50)													// ת��С�� 50 rmp ����ʾת��								
				Timer3_freq1 = 0;
		if(Timer4_freq2 < 50)
				Timer4_freq2 = 0;
		
		Tx_Buf[12] = Timer3_freq1 >> 8;
		Tx_Buf[13] = Timer3_freq1;
		Tx_Buf[14] = Timer4_freq2 >> 8;
		Tx_Buf[15] = Timer4_freq2;	
									
		Tx_Buf[19] = crc_calc(Tx_Buf, Tx_Len);							//	CRC��֤�����
	
#endif
}





