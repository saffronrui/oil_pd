//*********************************************//
// PT1000�����
//  2018.7.9@ruirui
//*********************************************//

#include "sys.h"
#include "pt1000.h"

// pt1000 AD����ֵ 0-4096����Ӧ����궨ֵ���ֶ��������
// ���ȸ��ݲ���ֵ���Ȼ��������ߣ����ؼ���ֵ
//--------------------------------------------------  �ĸ�ͨ��һ���Բ��ã���Ҫ��������
//---����ֵ				 820			1000		1200	 1400		1600	 1800			2000
//---�¶�ֵ				-45.7			0		  	51.6   103.9	157.1	 211.5		266.5	
//---AD3����ֵ
//---AD4����ֵ
//---AD5����ֵ
//---AD6����ֵ



float PT_Table[PT1000_TABLE_NUM] 	= {-45.7, 0.0, 51.6, 103.6, 157.1, 211.5, 266.5};
//u16		AD1_Table[PT1000_TABLE_NUM]	= {};		//AD�궨ֵ����
//u16		AD2_Table[PT1000_TABLE_NUM]	= {};
//u16		AD3_Table[PT1000_TABLE_NUM]	= {};
//u16		AD4_Table[PT1000_TABLE_NUM]	= {};

//float	PT1_para[PT1000_TABLE_NUM]	= {};		// �¶�����б������
//float	PT2_para[PT1000_TABLE_NUM]	= {};
//float	PT3_para[PT1000_TABLE_NUM]	= {};
//float	PT4_para[PT1000_TABLE_NUM]	= {};	

u16 Get_Temputure(u16 ad_sample)
{
		u8	i;
		float k;
		
		if( (ad_sample < PT_Table[0]) || (ad_sample > PT_Table[PT1000_TABLE_NUM-1]) )			// �������·�Χ
				 return 65535;
		
		for( i = 0; i < PT1000_TABLE_NUM; i++ ){
				 if( ad_sample >= PT_Table[i] )
						 break;
		}
		
		k = (PT_Table[i+1] - PT_Table[i]) / 10.;			//����ֶ�б��
		
		return (u16)( -50 + 10 * i + k * (ad_sample - PT_Table[i]) );
}


