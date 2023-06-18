#ifndef ADC_H
#define ADC_H

#define I16DACOEFK_VOUT  ((UINT16)21300)     //Q14����ѹ����
#define CurrentPeriod    30					 //msΪ��λ��30ms��30��

#define LENGTH_TBLTEMP_PORT_10K    ((UINT16)56)

#define AD_CalNum			8		//������λ�ƴ���>>2������������Ҹ�
#define AD_CalNum_2			3		//2^3 = 8�������Ǹ�����2�Ķ��ٴη�������λ��

#define AD_CalNum_Cur		32		//������λ�ƴ���>>5������������Ҹ�
#define AD_CalNum_Cur_2		5		//2^5 = 32�������Ǹ�����2�Ķ��ٴη�������λ��

//AD��������ö��
enum tagInfoForADCArray
{
    ADC_TEMP_EV1,                     	// �ⲿ�¶�
	ADC_TEMP_EV2,                     	// �ⲿ�¶�
    //ADC_VBC,                         	// ĸ�ߵ�ѹ
    ADC_TEMP_MOS1,                    	// MOS2�¶�
    //ADC_TEMP_MOS2,                    	// MOS1�¶�
    //ADC_CURR,                         	// ����
    //ADC_EXT_C1,                       	// ��1�ڵ�ص�ѹ
    //ADC_EXT_C2,                       	// ��2�ڵ�ص�ѹ

	ADC_NUM		                            // ADC number
};


#define Vbc_scale_16 	  31	  		//��ѹ�ɱ���ֵ
#define Vbc_scale_6 	  11	  		//��ѹ�ɱ���ֵ


extern INT32 g_i32ADCResult[ADC_NUM];             //ADC���ݻ���


void InitADC(void);
void App_AnlogCal(void);

#endif	/* ADC_H */
