#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

extern u16 Timer3_freq1;
extern u16 Timer4_freq2;
 
void TIM1_PWM_Init(u16 arr,u16 psc);
void TIM2_Cap_Init(u16 arr,u16 psc);
void TIM2_Config(void);
void TIM3_Counter_Config();
void GPIO_Counter_Config(void);

#endif
