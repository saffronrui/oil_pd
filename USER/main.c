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
#include "pt1000.h"

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
	 
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2	 
	
	delay_init();	    	 //��ʱ������ʼ��	  
	
	uart_init(19200);	 //���ڳ�ʼ��Ϊ19200
	
	MYDMA_Config(DMA1_Channel4,(u32)&USART1->DR,(u32)Tx_Buf,Tx_Len);//DMA1ͨ��4,����Ϊ����1,�洢��ΪTx_buf,����Ϊ sizeof(tx_buf)
	
	LED_Init();		 //��ʼ����LED��RELAY���Ӷ˿ڳ�ʼ��   
	
	AT24CXX_Init();			//IIC��ʼ�� 
//	AT24CXX_Write(version_addr,(u8*)Program_Version,sizeof(Program_Version));

//-------------------------------------------------------------------------------//	 
// timer1 ���pwm��������Ƶ�ʲɼ�����Ҫʱȥ��ע�ͼ���
// ��Ҫע�⣬ PA8 ������� pwm���˶˿��� mosfet ���ƶ˿ڸ���

	TIM1_PWM_Init(899,99); 			  //����Ƶ��PWMƵ��=72000/(8999+1)=8Khz, ��������pwm
																// Ƶ�ʲ�����Χ 800~9000
	TIM_SetCompare1(TIM1,70);
//-------------------------------------------------------------------------------//	
	
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
	
//	float_type.ff = 3.1415926;
//	AT24CXX_WriteOneByte(0, float_type.fchar[0]);
//	AT24CXX_WriteOneByte(1, float_type.fchar[1]);
//	AT24CXX_WriteOneByte(2, float_type.fchar[2]);
//	AT24CXX_WriteOneByte(3, float_type.fchar[3]);	
	

	 
	while(1)
	{
		Sci_Cmd_function();				//����������
		
//		f1_type.fchar[0] = AT24CXX_ReadOneByte(0);
//		f1_type.fchar[1] = AT24CXX_ReadOneByte(1);
//		f1_type.fchar[2] = AT24CXX_ReadOneByte(2);
//		f1_type.fchar[3] = AT24CXX_ReadOneByte(3);
//		printf("%f", f1_type.ff);
		
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
		
//		for( i = 3; i < 7; i++ ){
//				printf("%c", adcx[1] >> 8);
//				printf("%c", adcx[1]);
//		}	

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
	
		STM_ADC_DATA_f[0] =  1.51 * adcx[0]  * 3.3 * 10 / 4096;		//��������									-->PA7
		STM_ADC_DATA_f[1] =  adcx[1]*0.048197;										//12��ѹ����								-->PC0
		STM_ADC_DATA_f[2] =  adcx[2]*0.2789 - 0.7849 ;						//24��ѹ����								-->PC1

//		STM_ADC_DATA_f[3] =  adcx[3];														//�¶�1����									-->PC3
		STM_ADC_DATA_f[3] = Get_Temputure(adcx[3], 0);															//�¶�1����		
		STM_ADC_DATA_f[4] = Get_Temputure(adcx[4], 1);															//�¶�2����									-->PC2
		STM_ADC_DATA_f[5] = Get_Temputure(adcx[5], 2);															//�¶�3����									-->PA0
		STM_ADC_DATA_f[6] = Get_Temputure(adcx[6], 3);															//�¶�4����									-->PA1
		
		STM_ADC_DATA_f[7] =  adcx[7];					//���õ�ѹ����1  0-10V			-->PA2
		STM_ADC_DATA_f[8] =  adcx[8];					//���õ�ѹ����2  0-10V			-->PA3
	
		Fault_sta = Voltage_Current_Protection();			//�豸������������Ӧ������
	
	
		Tx_data_function();	
		
		delay_ms(200);
		IWDG_Feed();				//	ι��
	}											    
}	
 

char Voltage_Current_Protection(void)
{

		if( STM_ADC_DATA_f[0] > 20. ){			//��������
				//�ر�����mosfetͨ��
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
				return 0xBB;
		} 	
		
		if( (STM_ADC_DATA_f[2] > 260 ) || (STM_ADC_DATA_f[2] < 180) ){			// 24V��Դ��ѹ��ѹ����
				 //�ر�����moefetͨ��
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
//				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
//				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
				return 0xCC;
		}
		
		if( (STM_ADC_DATA_f[1] > 130 ) || (STM_ADC_DATA_f[1] < 105) ){			// 12V��Դ��ѹ��ѹ����
				 //�ر�����moefetͨ��
				 GPIO_ResetBits(GPIOA,GPIO_Pin_11);					  // mosfet1 turn-off
				 GPIO_ResetBits(GPIOA,GPIO_Pin_8); 						// mosfet2 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_2);   					// mosfet3 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_3);   				  // mosfet4 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_1);   					// mosfet5 turn-off
				 GPIO_ResetBits(GPIOE,GPIO_Pin_4);   				  // mosfet6 turn-off
			
			return	0xAF;
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
								LED1 = !LED1;
	//							if(crc_calc(Sci_cmd_buf, CMD_BUF_LEN - 1 ) == Sci_cmd_buf[CMD_BUF_LEN-1]);		// ���� crc ��֤��ʱ����Ҫȥ�����һ���ֽڣ����Գ���Ϊ CMD_BUF_LEN -1
										CMD_Val = Sci_cmd_buf[2];
								
								for( i = 0; i < CMD_BUF_LEN; i++ )			//����һ���������ս��ջ��������ȴ���һ������
										Sci_cmd_buf[i] = 0x00;
							}
		 
		switch(CMD_Val)
		{	
				case	0xE0: GPIO_SetBits(GPIOB,GPIO_Pin_4);      	break;				// ͣ������ 1 ����
				case	0xEA:	GPIO_ResetBits(GPIOB,GPIO_Pin_4);			break;				// ͣ������ 1 ��λ
				
				case	0xF0: GPIO_SetBits(GPIOB,GPIO_Pin_5);     	break;				// ͣ������ 2 ����
				case	0xFA:	GPIO_ResetBits(GPIOB,GPIO_Pin_5);			break;				// ͣ������ 2 ��λ
				
				case	0xA0: GPIO_SetBits(GPIOA,GPIO_Pin_11);	  	break;				// mosfet1 turn-on
				case	0xAA:	GPIO_ResetBits(GPIOA,GPIO_Pin_11);		break;				// mosfet1 turn-off
				
				case	0xB0:	GPIO_SetBits(GPIOA,GPIO_Pin_8);   		break;				// mosfet2 turn-on
				case	0xBA:	GPIO_ResetBits(GPIOA,GPIO_Pin_8); 		break;				// mosfet2 turn-off
				
				case	0xC0:	GPIO_SetBits(GPIOE,GPIO_Pin_2);	    	break;				// mosfet3 turn-on
				case	0xCA:	GPIO_ResetBits(GPIOE,GPIO_Pin_2);   	break;				// mosfet3 turn-off
				
				case	0xD0:	GPIO_SetBits(GPIOE,GPIO_Pin_3);		    break;				// mosfet4 turn-on
				case	0xDA:	GPIO_ResetBits(GPIOE,GPIO_Pin_3);   	break;				// mosfet4 turn-off
							
				case	0x80:	GPIO_SetBits(GPIOE,GPIO_Pin_1);	    	break;				// mosfet5 turn-on				//��ʱû�к���
				case	0x8A:	GPIO_ResetBits(GPIOE,GPIO_Pin_1);   	break;				// mosfet5 turn-off
				
				case	0x90:	GPIO_SetBits(GPIOE,GPIO_Pin_4);		    break;				// mosfet6 turn-on				//��ʱû�к���
				case	0x9A:	GPIO_ResetBits(GPIOE,GPIO_Pin_4);   	break;				// mosfet6 turn-off
			
				case	'V':	AT24CXX_Read(version_addr,datatemp,20);	printf("%s\n", datatemp);		break;				// ����汾�����ѯ
				default : break;
		}

}

void Tx_data_function(void)
{
		u8	frame_cnt;
		u16 freq1, freq2;
#if !Is_Debug 																		// �Ƿ�Ϊ����ģʽ
		
		frame_cnt = 0;
	
		Tx_Buf[frame_cnt++]  = 0xEE;														// ֡ͷ
		Tx_Buf[frame_cnt++]  = 'P';															// ��־λ
		Tx_Buf[frame_cnt++]  = Tx_Len;													// ���鳤��
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[0];								// �����ɼ�							
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[1];								// 12V ��ѹ�ɼ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[2] >> 8;						// 24V ��ѹ�ɼ����ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[2];								// 24V ��ѹ�ɼ����ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[3] >> 8;						// �¶Ȳɼ�1���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[3];								// �¶Ȳɼ�1���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[4] >> 8;						// �¶Ȳɼ�2���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[4];								// �¶Ȳɼ�2���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[5] >> 8;						// �¶Ȳɼ�3���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[5];								// �¶Ȳɼ�3���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[6] >> 8;						// �¶Ȳɼ�4���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[6];								// �¶Ȳɼ�4���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[7] >> 8;						// ����ADͨ��1���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[7];								// ����ADͨ��1���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[8] >> 8;						// ����ADͨ��2���ֽ�
		Tx_Buf[frame_cnt++]  = (u16)STM_ADC_DATA_f[8];								// ����ADͨ��2���ֽ�
		
		freq1 = Timer3_freq1 * 4.0020;		//	ת��ϵ����������׼ϵ��Ϊ 250ms һ���жϣ� *4����ñ������жϺ�����������
		freq2 = Timer4_freq2 * 4.0020;
		
				
		if(freq1 < 50)													// ת��С�� 50 rmp ����ʾת��								
				freq1 = 0;
		if(freq2 < 50)
				freq2 = 0;
		
		Tx_Buf[frame_cnt++] = freq1 >> 8;
		Tx_Buf[frame_cnt++] = freq1;
		Tx_Buf[frame_cnt++] = freq2 >> 8;
		Tx_Buf[frame_cnt++] = freq2;	
									
		Tx_Buf[Tx_Len-3] = Fault_sta;															//	����״̬��ʾ, 0XAF ��ʾû�й���
		Tx_Buf[Tx_Len-1] = crc_calc(Tx_Buf, Tx_Len);							//	CRC��֤�����
	
#endif
}





