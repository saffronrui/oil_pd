#ifndef __WDG_H
#define __WDG_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途

////////////////////////////////////////////////////////////////////////////////// 	  


void IWDG_Init(u8 prer,u16 rlr);
void IWDG_Feed(void);

 
#endif
