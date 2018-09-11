//*********************************************//
// PT1000查表函数
//  2018.7.9@ruirui
//*********************************************//

#include "sys.h"
#include "pt1000.h"

// pt1000 AD采样值 0-4096，对应电阻标定值，分段曲线拟合
// 首先根据采样值查表，然后拟合曲线，返回计算值
//--------------------------------------------------  四个通道一致性不好，需要单独计算
//---电阻值				 820			1000		1200	 		1400			1600	 		1800				2000      
//---温度值				-45.7			0		  	51.6   		103.9			157.1	 		211.5				266.5	
//---AD3采样值		 287			942     1576   		2102   		2490   		2763     		2943       
//---AD4采样值
//---AD5采样值
//---AD6采样值



float PT_Table[PT1000_TABLE_NUM] 	= {-45.7, 0.0, 51.6, 103.6, 157.1, 211.5, 266.5};

u16 	AD_Table[4][PT1000_TABLE_NUM] = {{287,   942, 1582, 2137,  2619,  3057,  3427}, 		// T1	AD标定值
																			 {287,   942, 1582, 2137,  2619,  3057,  3427},			// T2	AD标定值
																	     {287,   942, 1582, 2137,  2619,  3057,  3427},			// T3	AD标定值																							
																			 {287,   942, 1582, 2137,  2619,  3057,  3427}};		// T4	AD标定值												

//float	PT1_para[PT1000_TABLE_NUM]	= {};		// 温度曲线斜率数组
//float	PT2_para[PT1000_TABLE_NUM]	= {};
//float	PT3_para[PT1000_TABLE_NUM]	= {};
//float	PT4_para[PT1000_TABLE_NUM]	= {};	

u16 Get_Temputure(u16 ad_sample, u8 T_ch)
{
		u8	i;
		float k;
		
		if( (ad_sample < AD_Table[T_ch][0]) || (ad_sample > AD_Table[T_ch][PT1000_TABLE_NUM-1]) )			// 超出测温范围
				 return 65535;
		
		for( i = 0; i < PT1000_TABLE_NUM - 1; i++ ){
				 if( (ad_sample >= AD_Table[T_ch][i])&&(ad_sample < AD_Table[T_ch][i+1]) )
						 break;
		}
		
		k = (PT_Table[i+1] - PT_Table[i]) / (AD_Table[T_ch][i+1] - AD_Table[T_ch][i]);			//计算分段斜率
		
		return (u16)( PT_Table[i] + k * (ad_sample - AD_Table[T_ch][i]) + 55 );							// 返回实际计算温度值 +55 摄氏度
}


