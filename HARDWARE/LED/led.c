#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
// 油动多旋翼配电盒控制程序		
// LED控制文件
// 控制板两个 LED ： LED0 --> PB10
//									 LED1 --> PC12		
// LED_RELAY_INIT 函数，处理器端口初始化
// 输入值： void
// 返回值： void
////////////////////////////////////////////////////////////////////////////////// 	   

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_RELAY_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PA,PD端口时钟

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);
 GPIO_SetBits(GPIOC,GPIO_Pin_12);											 // turn off LED1
						
// GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_SetBits(GPIOB,GPIO_Pin_10);											 // turn off LED0					

}



