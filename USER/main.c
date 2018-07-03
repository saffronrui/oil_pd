//****************************************************************//
// �Ͷ����������˻����п��Ƴ���
// ������ stm32f103vet
// github: https://github.com/saffronrui/oil_pd
// ʹ�� github ʱ��Ҫע�⣺ ϵͳ�ļ����к����� OBJ �ļ��У����� clone ��Ŀ������֮����Ҫ�ֶ������յ� OBJ �ļ���
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

u8 Tx_data[34];
u8 Sci_cmd[3];
u8 Sci_cmd_sta;		// ���������־��= 0��ʾû�н��յ�����֡ͷ, = 1��ʾ������յ�֡ͷEB
									// = 2��ʾ���յ�����֡ͷ CD�� = 3��ʾ������յ�ʵ�������ַ�
									// = E0 ��ʾȫ�أ� = EF ��ʾȫ���� 

#define	ADC_CH_NUM	9		//ADC����ͨ��������

float	STM_ADC_DATA_f[ADC_CH_NUM];
float	STM_ADC_P[ADC_CH_NUM];
float STM_ADC_F[ADC_CH_NUM];

extern u8  TIM2CH1_CAPTURE_STA;		//���벶��״̬		    				
extern u16	TIM2CH1_CAPTURE_VAL;	//���벶��ֵ


char	Fault_sta = 0x00;

void Sci_Cmd_function(void);
void Tx_data_function(void);
char Voltage_Current_Protection(void);

 int main(void)
 { 
	u16 adcx[ADC_CH_NUM], addata;
	u8 i;
	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2	 
	
	delay_init();	    	 //��ʱ������ʼ��	  

	LED_Init();		 //��ʼ����LED��RELAY���Ӷ˿ڳ�ʼ��
 	GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);
	
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	Adc_Init();		  		 //ADC��ʼ��	    	    
	
	TIM1_PWM_Init(8999,0); 			  //����Ƶ��PWMƵ��=72000/(8999+1)=8Khz, ��������pwm
 //	TIM2_Cap_Init(0XFFFF,72-1);		//��1Mhz��Ƶ�ʼ��� 
	
	TIM2_Config();			// TIM2 ��ʱ�ж����ã�250ms �ж�
	TIM3_Counter_Config();		// TIM3����������
//	TIM4_Counter_Config();	// TIM4����������
	GPIO_Counter_Config();		// 	����˿�����
	 
	 
	STM_ADC_P[0] 	= 0.001204172; STM_ADC_F[0]  = 0.001537505;						//�����ɼ�CH1ϵ��
	STM_ADC_P[1] 	= 0.001206592; STM_ADC_F[1]  = 0.003148714;						//�����ɼ�CH2ϵ��
	STM_ADC_P[2] 	= 0.001206067; STM_ADC_F[2]  = 0.002076778;						//�����ɼ�CH3ϵ��
	STM_ADC_P[3] 	= 0.001205098; STM_ADC_F[3]  = 0.001673464;						//�����ɼ�CH4ϵ��
	STM_ADC_P[4] 	= 0.001206067; STM_ADC_F[4]  = 0.000870711;						//�����ɼ�CH5ϵ��
	STM_ADC_P[5] 	= 0.001205622; STM_ADC_F[5]  = 0.001238956;						//�����ɼ�CH6ϵ��
	STM_ADC_P[6] 	= 0.001203609; STM_ADC_F[6]  = 0.001804812;						//�����ɼ�CH7ϵ��
	STM_ADC_P[7] 	= 0.001205058; STM_ADC_F[7]  = 0.004016861;						//�����ɼ�CH8ϵ��
	STM_ADC_P[8] 	= 0.001208981; STM_ADC_F[8]  = 0.003190616;						//�����ɼ�CH9ϵ��

	 for(i = 0; i < 16; i++)
	{
		adcx[i] = 0;
		STM_ADC_DATA_f[i] = 0.0;
	}
	 
	IWDG_Init(4,625);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s
	
	 while(1)
	{
		Sci_Cmd_function();				//����������
		
		// ADC ����
		adcx[0]=Get_Adc_Average(ADC_Channel_0,10);					//��������
		adcx[1]=Get_Adc_Average(ADC_Channel_1,10);					//24��ѹ����
		adcx[2]=Get_Adc_Average(ADC_Channel_2,10);					//12��ѹ����
		adcx[3]=Get_Adc_Average(ADC_Channel_3,10);					//�¶�1����
		adcx[4]=Get_Adc_Average(ADC_Channel_4,10);					//�¶�2����
		adcx[5]=Get_Adc_Average(ADC_Channel_5,10);					//�¶�3����
		adcx[6]=Get_Adc_Average(ADC_Channel_6,10);					//�¶�4����
		adcx[7]=Get_Adc_Average(ADC_Channel_7,10);					//���õ�ѹ����1  0-10V
		adcx[8]=Get_Adc_Average(ADC_Channel_8,10);					//���õ�ѹ����2  0-10V

		// ADC �����������
		STM_ADC_DATA_f[0] =  adcx[0]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
		STM_ADC_DATA_f[1] =  adcx[1]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9
		STM_ADC_DATA_f[2] =  adcx[2]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
		STM_ADC_DATA_f[3] =  adcx[3]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9
		STM_ADC_DATA_f[4] =  adcx[4]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
		STM_ADC_DATA_f[5] =  adcx[5]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9	
		STM_ADC_DATA_f[6] =  adcx[6]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
		STM_ADC_DATA_f[7] =  adcx[7]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9	
		STM_ADC_DATA_f[8] =  adcx[8]  * STM_ADC_P[8] + STM_ADC_F[8] ;					//��������ͨ��10
	
		Fault_sta = Voltage_Current_Protection();			//�豸������������Ӧ������
	
	
		Tx_data_function();
	
		for( i = 0; i < 34; i++ )							//	�������
		{
						printf("%c", Tx_data[i]);
		}
		
//		LED0=!LED0;		//��ת���ư�LED
		delay_ms(10);
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
	
		if( Sci_cmd_buf[0] == 0xEB )
				if( Sci_cmd_buf[1] == 'E' )
							{
								if(crc_calc(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// ���� crc ��֤��ʱ����Ҫȥ�����һ���ֽڣ����Գ���Ϊ CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//����һ���������ս��ջ��������ȴ���һ������
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
		
	  Tx_data[4]  = STM_ADC_DATA_f[1];						// ����1����
		Tx_data[5]  = STM_ADC_DATA_f[2];						// ����1����
		Tx_data[6]  = STM_ADC_DATA_f[3];						// ����1����
		Tx_data[7]  = STM_ADC_DATA_f[0];						// ����1����
		Tx_data[8]  = STM_ADC_DATA_f[4];						// ����1����
				
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



