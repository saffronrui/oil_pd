//*********************************************//
// PT1000查表函数
//  2018.7.9@ruirui
//*********************************************//

#include "sys.h"
#include "pt1000.h"

// pt1000 AD采样值 0-4096，对应电阻标定值，分段曲线拟合
// 首先根据采样值查表，然后拟合曲线，返回计算值
//--------------------------------------------------  四个通道一致性不好，需要单独计算
//---电阻值				 820			1000		1200	 1400		1600	 1800			2000
//---温度值				-45.7			0		  	51.6   103.9	157.1	 211.5		266.5	
//---AD3采样值
//---AD4采样值
//---AD5采样值
//---AD6采样值



float PT_Table[PT1000_TABLE_NUM] 	= {-45.7, 0.0, 51.6, 103.6, 157.1, 211.5, 266.5};
//u16		AD1_Table[PT1000_TABLE_NUM]	= {};		//AD标定值数组
//u16		AD2_Table[PT1000_TABLE_NUM]	= {};
//u16		AD3_Table[PT1000_TABLE_NUM]	= {};
//u16		AD4_Table[PT1000_TABLE_NUM]	= {};

//float	PT1_para[PT1000_TABLE_NUM]	= {};		// 温度曲线斜率数组
//float	PT2_para[PT1000_TABLE_NUM]	= {};
//float	PT3_para[PT1000_TABLE_NUM]	= {};
//float	PT4_para[PT1000_TABLE_NUM]	= {};	

u16 Get_Temputure(u16 ad_sample)
{
		u8	i;
		float k;
		
		if( (ad_sample < PT_Table[0]) || (ad_sample > PT_Table[PT1000_TABLE_NUM-1]) )			// 超出测温范围
				 return 65535;
		
		for( i = 0; i < PT1000_TABLE_NUM; i++ ){
				 if( ad_sample >= PT_Table[i] )
						 break;
		}
		
		k = (PT_Table[i+1] - PT_Table[i]) / 10.;			//计算分段斜率
		
		return (u16)( -50 + 10 * i + k * (ad_sample - PT_Table[i]) );
}


