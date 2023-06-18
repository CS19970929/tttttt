#include "main.h"


const UINT16 AFE_SCV[10] = {22,33,44,67,89,111,133,155,178,200};    //��·������ѹ����λmv
const UINT16 AFE_SCT[10] = {50,100,200,400,400,400,400,400,400,400};


/* �ҳ�Ҫд��AFE�Ĵ�����ֵ������1����ǰҪд��ֵ������2��AFE�����б�ĵ�ַ */
UINT8 Choose_Right_Value(UINT16 cur_Value, const UINT16 *AFE_list) {
	UINT8 i = 0;
	for(i = 0; i < 10; i++) {
		if(cur_Value <= AFE_list[i]) {
			break;
		}
	}
    i=i>=10?9:i;
	return i;
}


//�ڿ�����Sci(���ú͸�λ��������)�����ε��ñ��
void InitShortCur(void) {
    UINT16 temp = 0;

    OtherElement.u16CS_Cur_CHGmax = 2000*OtherElement.u16Sys_CS_Res_Num/OtherElement.u16Sys_CS_Res;
    OtherElement.u16CS_Cur_DSGmax = 2000*OtherElement.u16Sys_CS_Res_Num/OtherElement.u16Sys_CS_Res;

    /* ��·��ʱ */
	temp = Choose_Right_Value(OtherElement.u16CBC_DelayT/10,AFE_SCT);
    OtherElement.u16CBC_DelayT = AFE_SCT[temp]*10;         //�޸��������õ�ֵ���ӽ����Ǹ�

    Registers_AFE1.Protect1.Protect1Bit.SCD_DELAY = temp>=3?3:temp;

	/* ��·��ѹ */
    temp = OtherElement.u16CBC_Cur_DSG/10;      //A
	temp = temp*OtherElement.u16Sys_CS_Res/OtherElement.u16Sys_CS_Res_Num; //��ǰ��Ӧ����mV
	temp = Choose_Right_Value(temp,AFE_SCV);
    //�޸��������õ�ֵ���ӽ����Ǹ�
    OtherElement.u16CBC_Cur_DSG = AFE_SCV[temp]*OtherElement.u16Sys_CS_Res_Num/OtherElement.u16Sys_CS_Res; 
    OtherElement.u16CBC_Cur_DSG *= 10;          //��ֹ���������

    if(temp <= 1) {
        Registers_AFE1.Protect1.Protect1Bit.RSNS = 0;
        Registers_AFE1.Protect1.Protect1Bit.SCD_THRESH = temp;
    }
    else {
        Registers_AFE1.Protect1.Protect1Bit.RSNS = 1;
        Registers_AFE1.Protect1.Protect1Bit.SCD_THRESH = temp - 2;
    }

    //Ӳ�����������������
	Registers_AFE1.Protect2.Protect2Bit.OCD_DELAY = OCD_DELAY_1280ms;    //8ms
	Registers_AFE1.Protect2.Protect2Bit.OCD_THRESH = OCD_THRESH_100mV_50mV;  //20A
	
    I2CWriteRegisterByteWithCRC(DEVICE_ADDR_AFE1, PROTECT1, Registers_AFE1.Protect1.Protect1Byte);
    I2CWriteRegisterByteWithCRC(DEVICE_ADDR_AFE1, PROTECT2, Registers_AFE1.Protect2.Protect2Byte);

    // ����֮����Ҫ������������
    // MCU_RESET();    
}
