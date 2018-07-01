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

u8 Tx_data[34];
u8 Sci_cmd[3];
u8 Sci_cmd_sta;		// ���������־��= 0��ʾû�н��յ�����֡ͷ, = 1��ʾ������յ�֡ͷEB
									// = 2��ʾ���յ�����֡ͷ CD�� = 3��ʾ������յ�ʵ�������ַ�
									// = E0 ��ʾȫ�أ� = EF ��ʾȫ���� 

u16 MCP3208_N1_DATA[8];
u16	time_cnt;
float	STM_ADC_DATA_f[16];
float	STM_ADC_P[16];
float STM_ADC_F[16];
float MCP3208_N1_P[10];
float MCP3208_N1_F[10];
float MCP3208_N1_DATA_f[8];
//const float MCP3208_N1_DATA_offset[8];


#if MCP3208_N2_ENABLE
u16 MCP3208_N2_DATA[8];
const float MCP3208_N2_DATA_f[8];
const float MCP3208_N2_DATA_offset[8];
void MCP3208_N2_ADC(void);
#endif

void MCP3208_N1_ADC(void);
void Sci_Cmd_function(void);
void Tx_data_function(void);

 int main(void)
 { 
	u16 adcx[16], addata;
	float temp[16];
	u8 i;
	 
	delay_init();	    	 //��ʱ������ʼ��	  

	LED_RELAY_Init();		 //��ʼ����LED��RELAY���Ӷ˿ڳ�ʼ��
 	GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);
	uart_init(115200);	 //���ڳ�ʼ��Ϊ9600
	Adc_Init();		  		 //ADC��ʼ��	    	    
	
	SPI_ADC_Init();
	#if	MCP3208_N2_ENABLE
	SPI2_Init();					//��ʼ������MCP3208
  #endif

	MCP3208_N1_P[0] = 0.021701085;   MCP3208_N1_F[0] = -1.827192043;				//��ѹ�ɼ�CH1 ϵ��
	MCP3208_N1_P[1] = 0.022019266;   MCP3208_N1_F[1] = -2.190679817;				//��ѹ�ɼ�CH2 ϵ��
	MCP3208_N1_P[2] = 0.021699638;   MCP3208_N1_F[2] = -1.867435262;				//��ѹ�ɼ�CH3 ϵ��
	MCP3208_N1_P[3] = 0.021937477;   MCP3208_N1_F[3] = -2.151320658;				//��ѹ�ɼ�CH4 ϵ��
	MCP3208_N1_P[4] = 0.016553241;   	 MCP3208_N1_F[4] = 0.125830483;				//��ѹ�ɼ�CH5 ϵ��, stm32 ad�ɼ�
	MCP3208_N1_P[5] = 0.016621607;   	 MCP3208_N1_F[5] = 0.118736842;				//��ѹ�ɼ�CH6 ϵ����stm32 ad�ɼ�
	MCP3208_N1_P[6] = 0.021315986;   MCP3208_N1_F[6] = -1.461105684;				//��ѹ�ɼ�CH7 ϵ��
	MCP3208_N1_P[7] = 0.021582734;   MCP3208_N1_F[7] = -1.696230216;				//��ѹ�ɼ�CH8 ϵ��
	MCP3208_N1_P[8] = 0.021316163;   MCP3208_N1_F[8] = -1.462534636;				//��ѹ�ɼ�CH9 ϵ��
	MCP3208_N1_P[9] = 0.021315808;   MCP3208_N1_F[9] = -1.396929307;				//��ѹ�ɼ�CH10 ϵ��

	STM_ADC_P[0] 	= 0.001204172; STM_ADC_F[0]  = 0.001537505;						//�����ɼ�CH1ϵ��
	STM_ADC_P[1] 	= 0.001206592; STM_ADC_F[1]  = 0.003148714;						//�����ɼ�CH2ϵ��
	STM_ADC_P[2] 	= 0.001206067; STM_ADC_F[2]  = 0.002076778;						//�����ɼ�CH3ϵ��
	STM_ADC_P[3] 	= 0.001205098; STM_ADC_F[3]  = 0.001673464;						//�����ɼ�CH4ϵ��
	STM_ADC_P[4] 	= 0.001206067; STM_ADC_F[4]  = 0.000870711;						//�����ɼ�CH5ϵ��
	STM_ADC_P[5] 	= 0.001205622; STM_ADC_F[5]  = 0.001238956;						//�����ɼ�CH6ϵ��
	STM_ADC_P[6] 	= 0.001203609; STM_ADC_F[6]  = 0.001804812;						//�����ɼ�CH7ϵ��
	STM_ADC_P[7] 	= 0.001205058; STM_ADC_F[7]  = 0.004016861;						//�����ɼ�CH8ϵ��
	STM_ADC_P[8] 	= 0.001208981; STM_ADC_F[8]  = 0.003190616;						//�����ɼ�CH9ϵ��
	STM_ADC_P[9] 	= 0.001208008; STM_ADC_F[9]  = 0.002685352;						//�����ɼ�CH10ϵ��
	STM_ADC_P[10] = .0; STM_ADC_F[10] = .0;
	STM_ADC_P[11] = .0; STM_ADC_F[11] = .0;
	STM_ADC_P[12] = .0; STM_ADC_F[12] = .0;
	STM_ADC_P[13] = .0; STM_ADC_F[13] = .0;
	STM_ADC_P[14] = .0; STM_ADC_F[14] = .0;
	STM_ADC_P[15] = .0; STM_ADC_F[15] = .0;

	 for(i = 0; i < 16; i++)
	{
		temp[i] = 0.;
		adcx[i] = 0;
		STM_ADC_DATA_f[i] = 0.0;
	}
	for( i =0 ; i < 8; i++ ) 
	{
		MCP3208_N1_DATA_f[i] = 0.0;
		MCP3208_N1_DATA[i] = 0;
	}
	addata = 0;
	time_cnt = 0;
	 
	IWDG_Init(4,625);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s
	
	 while(1)
	{
		Sci_Cmd_function();				//����������
		
		adcx[0]=Get_Adc_Average(ADC_Channel_0,10);					//��������ͨ��10
		adcx[1]=Get_Adc_Average(ADC_Channel_1,10);					//��������ͨ��9
		adcx[2]=Get_Adc_Average(ADC_Channel_2,10);					//��ѹ����ͨ��5	
		adcx[3]=Get_Adc_Average(ADC_Channel_3,10);					//��ѹ����ͨ��6
		adcx[8]=Get_Adc_Average(ADC_Channel_8,10);					//��������ͨ��7
		adcx[9]=Get_Adc_Average(ADC_Channel_9,10);					//��������ͨ��6
		adcx[10]=Get_Adc_Average(ADC_Channel_10,10);				//��������ͨ��4
		adcx[11]=Get_Adc_Average(ADC_Channel_11,10);				//��������ͨ��1
		adcx[12]=Get_Adc_Average(ADC_Channel_12,10);				//��������ͨ��2
		adcx[13]=Get_Adc_Average(ADC_Channel_13,10);			  //��������ͨ��3
		adcx[14]=Get_Adc_Average(ADC_Channel_14,10);				//��������ͨ��5
		adcx[15]=Get_Adc_Average(ADC_Channel_15,10);				//��������ͨ��8
		
		STM_ADC_DATA_f[0] =  adcx[0]  * STM_ADC_P[9] + STM_ADC_F[9] ;					//��������ͨ��10
		STM_ADC_DATA_f[1] =  adcx[1]  * STM_ADC_P[8] + STM_ADC_F[8];					//��������ͨ��9
		STM_ADC_DATA_f[2] =  adcx[2]  * MCP3208_N1_P[4] + MCP3208_N1_F[4] ;		//��ѹ����ͨ��5	,12V���ز���
		STM_ADC_DATA_f[3] =  adcx[3]  * MCP3208_N1_P[5] + MCP3208_N1_F[5] ;		//��ѹ����ͨ��6������ͨ��
		STM_ADC_DATA_f[8] =  adcx[8]  * STM_ADC_P[6] + STM_ADC_F[6];				//��������ͨ��7
		STM_ADC_DATA_f[9] =  adcx[9]  * STM_ADC_P[5] + STM_ADC_F[5];				//��������ͨ��6
		STM_ADC_DATA_f[10] = adcx[10] * STM_ADC_P[3] + STM_ADC_F[3];				//��������ͨ��4
		STM_ADC_DATA_f[11] = adcx[11] * STM_ADC_P[0] + STM_ADC_F[0] ;				//��������ͨ��1
		STM_ADC_DATA_f[12] = adcx[12] * STM_ADC_P[1] + STM_ADC_F[1] ;				//��������ͨ��2
		STM_ADC_DATA_f[13] = adcx[13] * STM_ADC_P[2] + STM_ADC_F[2];				//��������ͨ��3
		STM_ADC_DATA_f[14] = adcx[14] * STM_ADC_P[4] + STM_ADC_F[4];				//��������ͨ��5
		STM_ADC_DATA_f[15] = adcx[15] * STM_ADC_P[7] + STM_ADC_F[7];				//��������ͨ��8		
		
		MCP3208_N1_ADC();
		
		MCP3208_N1_DATA_f[0] = MCP3208_N1_DATA[0] * MCP3208_N1_P[6] + MCP3208_N1_F[6];				//��ѹ�ɼ�ͨ��7
		MCP3208_N1_DATA_f[1] = MCP3208_N1_DATA[1] * MCP3208_N1_P[8] + MCP3208_N1_F[8];				//��ѹ�ɼ�ͨ��9
		MCP3208_N1_DATA_f[2] = MCP3208_N1_DATA[2] * MCP3208_N1_P[7] + MCP3208_N1_F[7];				//��ѹ�ɼ�ͨ��8
		MCP3208_N1_DATA_f[3] = MCP3208_N1_DATA[3] * MCP3208_N1_P[9] + MCP3208_N1_F[9];				//��ѹ�ɼ�ͨ��10
		MCP3208_N1_DATA_f[4] = MCP3208_N1_DATA[4] * MCP3208_N1_P[3] + MCP3208_N1_F[3];				//��ѹ�ɼ�ͨ��4
		MCP3208_N1_DATA_f[5] = MCP3208_N1_DATA[5] * MCP3208_N1_P[0] + MCP3208_N1_F[0];				//��ѹ�ɼ�ͨ��1
		MCP3208_N1_DATA_f[6] = MCP3208_N1_DATA[6] * MCP3208_N1_P[1] + MCP3208_N1_F[1];				//��ѹ�ɼ�ͨ��2
		MCP3208_N1_DATA_f[7] = MCP3208_N1_DATA[7] * MCP3208_N1_P[2] + MCP3208_N1_F[2];				//��ѹ�ɼ�ͨ��3

		Tx_data_function();
	
		for( i = 0; i < 34; i++ )							//	�������
		{
						printf("%c", Tx_data[i]);
		}
		
		LED0=!LED0;		//��ת���ư�LED
		delay_ms(10);
		IWDG_Feed();				//	ι��
	}											    
}	
 


void Sci_Cmd_function(void)				//����������
{
		 if( Sci_cmd_sta == 3 )
		 {
				 switch(Sci_cmd[2])
				 {
					 case 0xE0: GPIO_SetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);  GPIO_SetBits(GPIOB,GPIO_Pin_5);   break;
					 case 0xEF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14|GPIO_Pin_15);GPIO_ResetBits(GPIOB,GPIO_Pin_5);	break;
					 case	0xA0: GPIO_SetBits(GPIOC,GPIO_Pin_14);	  break;
					 case	0xAF:	GPIO_ResetBits(GPIOC,GPIO_Pin_14);	break;
					 case	0xB0:	GPIO_SetBits(GPIOC,GPIO_Pin_15);   	break;
					 case	0xBF:	GPIO_ResetBits(GPIOC,GPIO_Pin_15); 	break;
					 case	0xC0:	GPIO_SetBits(GPIOB,GPIO_Pin_5);	    break;
					 case	0xCF:	GPIO_ResetBits(GPIOB,GPIO_Pin_5);   break;					 
				 }
			Sci_cmd_sta = 0;			//	������һ�����׼����ʼ�´ν���
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
		
	  Tx_data[4]  = STM_ADC_DATA_f[11];						// ����1����
		Tx_data[5]  = STM_ADC_DATA_f[12];						// ����1����
		Tx_data[6]  = STM_ADC_DATA_f[13];						// ����1����
		Tx_data[7]  = STM_ADC_DATA_f[10];						// ����1����
		Tx_data[8]  = STM_ADC_DATA_f[14];						// ����1����
		Tx_data[9]  = STM_ADC_DATA_f[9];						// ����1����
		Tx_data[10] = STM_ADC_DATA_f[8];						// ����1����
		Tx_data[11] = STM_ADC_DATA_f[15];						// ����1����
		Tx_data[12] = STM_ADC_DATA_f[1] * 10;				// ����1����
		
	  temp = 0;
		temp = MCP3208_N1_DATA_f[5] * 10;
		Tx_data[13] = temp;
		Tx_data[14] = temp >> 8;	
		
		temp = 0;
		temp = MCP3208_N1_DATA_f[6] * 10;
		Tx_data[15] = temp;
		Tx_data[16] = temp >> 8;
				
		temp = 0;
		temp = MCP3208_N1_DATA_f[7] * 10;
		Tx_data[17] = temp;
		Tx_data[18] = temp >> 8;
		
		temp = 0;
		temp = MCP3208_N1_DATA_f[4] * 10;
		Tx_data[19] = temp;
		Tx_data[20] = temp >> 8;
		
		temp = 0;
		temp = STM_ADC_DATA_f[2] * 10;
		Tx_data[21] = temp;
		Tx_data[22] = temp >> 8;

		temp = 0;
		temp = STM_ADC_DATA_f[3] * 10;
		Tx_data[23] = temp;
		Tx_data[24] = temp >> 8;
		
		temp = 0;
		temp = MCP3208_N1_DATA_f[0] * 10;
		Tx_data[25] = temp;
		Tx_data[26] = temp >> 8;
		
		temp = 0;
		temp = MCP3208_N1_DATA_f[2] * 10;
		Tx_data[27] = temp;
		Tx_data[28] = temp >> 8;
		
		temp = 0;
		temp = MCP3208_N1_DATA_f[1] * 10;
		Tx_data[29] = temp;
		Tx_data[30] = temp >> 8;

		temp = 0;
		temp = MCP3208_N1_DATA_f[3] * 10;
		Tx_data[31] = temp;
		Tx_data[32] = temp >> 8;

		CRC_Byte = 0x00;
		for( i = 0; i< 33; i++ )
		{
				CRC_Byte ^= Tx_data[i];
		}
		Tx_data[33] = CRC_Byte;
}

///////////////////////////////////////////
//MCP3208_N1_ADC()
//ͨ�� SPI �ӿ���MCP3208ͨ�ţ���ȡ0-7ͨ��ADC�������
//���ݱ�����MCP3208_N1_DATA[]�У��޷���ֵ
///////////////////////////////////////////
void MCP3208_N1_ADC(void)	
{
//	GPIO_InitTypeDef GPIO_InitStructure;
	u8 temp1,	temp2 , i;
	
	for( i = 0; i < 8; i++ )					//	���������
			 MCP3208_N1_DATA[i] = 0;
	
	for( i = 0; i < 10; i++ )					// 10�β����������
	{
			temp1 = 0; 
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x06 );
			temp1 = SPI1_ReadWriteByte( 0x00 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[0] += ((temp1 << 8) + temp2) & 0x0fff ;
			__nop();
			GPIO_SetBits(GPIOA, GPIO_Pin_4);

			temp1 = 0;	
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x06 );
			temp1 = SPI1_ReadWriteByte( 0x40 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[1] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);
			
			temp1 = 0;
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x06 );
			temp1 = SPI1_ReadWriteByte( 0x80 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[2] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);

			temp1 = 0;
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			 __nop();
			temp1 = SPI1_ReadWriteByte( 0x06 );
			temp1 = SPI1_ReadWriteByte( 0xC0 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[3] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);

			temp1 = 0;	
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x07 );
			temp1 = SPI1_ReadWriteByte( 0x00 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[4] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);

			temp1 = 0;
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x07 );
			temp1 = SPI1_ReadWriteByte( 0x40 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[5] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);
			
			temp1 = 0;	
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x07 );
			temp1 = SPI1_ReadWriteByte( 0x80 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[6] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);

			temp1 = 0;	
			temp2 = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
			__nop();
			temp1 = SPI1_ReadWriteByte( 0x07 );
			temp1 = SPI1_ReadWriteByte( 0xC0 );
			temp2 = SPI1_ReadWriteByte( 0x00 );
			MCP3208_N1_DATA[7] += ((temp1 << 8) + temp2) & 0x0fff ;
			GPIO_SetBits(GPIOA, GPIO_Pin_4);	
	}

	for( i = 0; i < 8; i++ )				//	��ƽ��ֵ
			 MCP3208_N1_DATA[i] = MCP3208_N1_DATA[i] / 10;
	
}




