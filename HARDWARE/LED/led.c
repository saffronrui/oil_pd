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
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PA,PD端口时钟

 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_ResetBits(GPIOB,GPIO_Pin_10);											 // turn off LED1
						
// GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);
 GPIO_ResetBits(GPIOC,GPIO_Pin_12);											 // turn off LED0					

 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);			//	RELAY1 停车开关1
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;						 // RELAY2 停车开关2
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 
 GPIO_ResetBits(GPIOB,GPIO_Pin_4);											 // turn off relay1
 GPIO_ResetBits(GPIOB,GPIO_Pin_5);											 // turn off relay2
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;						 //mosfet3, mosfet4 控制端口
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);
 
 GPIO_ResetBits(GPIOE,GPIO_Pin_2);											 // turn off mosfet3
 GPIO_ResetBits(GPIOE,GPIO_Pin_3);											 // turn off mosfet4
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_11;						 //mosfet1, mosfet2 控制端口
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);
 
 GPIO_ResetBits(GPIOA,GPIO_Pin_11);											 // turn off mosfet1
 GPIO_ResetBits(GPIOA,GPIO_Pin_8);											 // turn off mosfet2
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4;						 //mosfet5, mosfet6 控制端口
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;		 //IO口速度为10MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);
 
 GPIO_ResetBits(GPIOE,GPIO_Pin_1);											 // turn off mosfet3
 GPIO_ResetBits(GPIOE,GPIO_Pin_4);											 // turn off mosfet4
}



