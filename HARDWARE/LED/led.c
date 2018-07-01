#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
// �Ͷ����������п��Ƴ���		
// LED�����ļ�
// ���ư����� LED �� LED0 --> PB10
//									 LED1 --> PC12		
// LED_RELAY_INIT �������������˿ڳ�ʼ��
// ����ֵ�� void
// ����ֵ�� void
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_RELAY_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PA,PD�˿�ʱ��

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO���ٶ�Ϊ10MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);
 GPIO_SetBits(GPIOC,GPIO_Pin_12);											 // turn off LED1
						
// GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO���ٶ�Ϊ10MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_SetBits(GPIOB,GPIO_Pin_10);											 // turn off LED0					

}



