//*********************************************//
// PT1000�����
//  2018.7.9@ruirui
//*********************************************//

#include "sys.h"
#include "pt1000.h"

// pt1000 AD����ֵ 0-4096����Ӧ����궨ֵ���ֶ��������
// ���ȸ��ݲ���ֵ���Ȼ��������ߣ����ؼ���ֵ
//  

float PT_Table[PT1000_TABLE_NUM] = {803.063,200};

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


