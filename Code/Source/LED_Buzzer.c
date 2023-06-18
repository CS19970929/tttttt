#include "main.h"

//LED_COMMAND LED_Command = LED_STARTUP;
LED_MODE Led_Mode;
LED_COMMAND LED_Command;
UINT8 gu8_MosOpenFlag = 0;

void LED_StartUp(void) {
	static UINT8 su8_Tcnt = 0;

	++su8_Tcnt;
	if(su8_Tcnt < 10) {			//��1s
		MCUO_LED_RED = LED_OPEN;
		MCUO_LED_GREEN = LED_CLOSE;
		MCUO_BUZZER = LED_CLOSE;
	}
	else if(su8_Tcnt < 20) {	//��1s
		MCUO_LED_RED = LED_CLOSE;
		MCUO_LED_GREEN = LED_OPEN;
		MCUO_BUZZER = LED_CLOSE;
	}
	else if(su8_Tcnt < 30) {	//��1s�ҽ�1s
		MCUO_LED_RED = LED_CLOSE;
		MCUO_LED_GREEN = LED_OPEN;
		MCUO_BUZZER = LED_OPEN;
	}
	else {
		//su8_Tcnt = 0;			//ֻ������ʱ����������Ż��ٴ�ִ��һ��
		Led_Mode.all = 0;
		LED_Command = LED_FAULT_MONITOR;
	}
}


//2��״������100%-0%����0-100%
void LED_SocLow(void) {
	static UINT16 su16_Tcnt = 0;
	static UINT16 su16_TwinkleCnt = 0;
	static UINT16 su16_Buzzer3minFlag = 0;
	static UINT16 su16_Buzzer15minFlag = 0;

	if(!MCUI_EMERGENCY_KEY) {
		LED_Command = LED_RESCUE_PIVOT;
		MCUO_LED_RED = 0;
		MCUO_LED_GREEN = 0;
		MCUO_BUZZER = 0;
		su16_Tcnt = 0;
		su16_TwinkleCnt = 0;
		su16_Buzzer3minFlag = 0;
		su16_Buzzer15minFlag = 0;
		return;
	}

	if(Led_Mode.bits.b1_LoadErr || Led_Mode.bits.b1_BatteryErr) {
		/*
		//���ﲻ�����ⲿ�豸������������Ȼ���������������ֲ���������
		MCUO_LED_RED = 0;
		MCUO_LED_GREEN = 1;
		MCUO_BUZZER = 0;
		*/
		su16_Tcnt = 0;
		su16_TwinkleCnt = 0;
		su16_Buzzer3minFlag = 0;
		su16_Buzzer15minFlag = 0;
		Led_Mode.bits.b1_SocLow = 0;	//����������ж�
		return;
	}

	if(g_stCellInfoReport.SocElement.u16Soc < 50 || Led_Mode.bits.b1_SocLow) {
		Led_Mode.bits.b1_SocLow = 1;	
		if(++su16_TwinkleCnt >= 5) {
			su16_TwinkleCnt = 0;
			MCUO_LED_RED = !MCUO_LED_RED;
			MCUO_LED_GREEN = MCUO_LED_RED;		//��յ��ͬ��
		}
		
		if(g_stCellInfoReport.SocElement.u16Soc > 30) {
			if(!su16_Buzzer3minFlag) {
				if(++su16_Tcnt < 3*600) {	//3min
					MCUO_BUZZER = 1;
				}
				else {
					su16_Tcnt = 0;
					MCUO_BUZZER = 0;
					su16_Buzzer3minFlag = 1;
				}
			}
		}
		else {
			if(!su16_Buzzer15minFlag) {
				if(++su16_Tcnt < 15*600) {	//15min
					MCUO_BUZZER = 1;
				}
				else {
					su16_Tcnt = 0;
					MCUO_BUZZER = 0;
					su16_Buzzer15minFlag = 1;
				}
			}
			if(!su16_Buzzer3minFlag)su16_Buzzer3minFlag = 1;	//�տ����ǵ���30%��ʱ��
		}
	}

	if(g_stCellInfoReport.SocElement.u16Soc > 55) {	//�ָ��������̵Ƴ���
		MCUO_LED_RED = LED_CLOSE;
		MCUO_LED_GREEN = LED_OPEN;
		MCUO_BUZZER = LED_CLOSE;
		su16_Tcnt = 0;
		su16_TwinkleCnt = 0;
		su16_Buzzer3minFlag = 0;
		su16_Buzzer15minFlag = 0;
		Led_Mode.bits.b1_SocLow = 0;
	}
}


void LED_BatteryErr(void) {
	if(!MCUI_EMERGENCY_KEY) {
		LED_Command = LED_RESCUE_PIVOT;
		MCUO_LED_RED = 0;
		MCUO_BUZZER = 0;
		MCUO_LED_GREEN = 0;
		return;
	}
	
	if(Led_Mode.bits.b1_LoadErr) {
		Led_Mode.bits.b1_BatteryErr = 0;
		return;
	}

	switch(Led_Mode.bits.b1_BatteryErr) {
		case 0:
			if((g_stCellInfoReport.unMdlFault_Third.all&0xEFCF) != 0) {	//������쳣����Ƴ����������������̵���
				MCUO_LED_RED = 1;
				MCUO_BUZZER = 1;
				MCUO_LED_GREEN = 0;
				Led_Mode.bits.b1_BatteryErr = 1;
			}
			break;
			
		case 1:
			if((g_stCellInfoReport.unMdlFault_Third.all&0xEFCF) == 0) {	//��ԭ
				MCUO_LED_RED = LED_CLOSE;
				MCUO_BUZZER = LED_CLOSE;
				MCUO_LED_GREEN = LED_CLOSE;
				Led_Mode.bits.b1_BatteryErr = 0;
			}
			break;
			
		default:
			break;
	}

	/*
	//��������д�����������⣬���ǵ�switch���
	if((g_stCellInfoReport.unMdlFault.all&0xEFCF) != 0) {	//������쳣����Ƴ����������������̵���
		MCUO_LED_RED = 1;
		MCUO_BUZZER = 1;
		MCUO_LED_GREEN = 0;
		Led_Mode.bits.b1_BatteryErr = 1;
	}
	else {								//��ԭ
		MCUO_LED_RED = LED_CLOSE;
		MCUO_BUZZER = LED_CLOSE;
		MCUO_LED_GREEN = LED_CLOSE;
		Led_Mode.bits.b1_BatteryErr = 0;
	}
	*/
}


//�����쳣�����������ť����������ԭ����������
//�������ۣ�������λ��ȡ������
void LED_LoadErr(void) {
	static UINT16 su16_TwinkleCnt = 0;

	if(!MCUI_EMERGENCY_KEY) {
		LED_Command = LED_RESCUE_PIVOT;

		su16_TwinkleCnt = 0;
		MCUO_LED_RED = 0;
		MCUO_LED_GREEN = 0;
		MCUO_BUZZER = 0;
		return;
	}


	switch(Led_Mode.bits.b1_LoadErr) {
		case 0:
			if(g_stCellInfoReport.unMdlFault_Third.all&0x0030) {	//�����쳣(��������·)�����������������
				Led_Mode.bits.b1_LoadErr = 1;
			}
			break;
			
		case 1:
			if(++su16_TwinkleCnt >= 5) {
				su16_TwinkleCnt =0;
				MCUO_LED_RED = !MCUO_LED_RED;
			}
			MCUO_BUZZER = 1;
			MCUO_LED_GREEN = 0;

			if(0) {		//��ԭ��ֻ��ͨ��������ԭ
				su16_TwinkleCnt = 0;
				MCUO_LED_RED = 0;
				MCUO_LED_GREEN = 0;
				MCUO_BUZZER = 0;
				Led_Mode.bits.b1_LoadErr = 0;

			}
			break;
			
		default:
			break;
	}
}


void LED_Rescue_Pivot(void) {
	static UINT16 su16_Tcnt = 0;
	static UINT16 su16_TwinkleCnt = 0;

	if(!MCUI_EMERGENCY_KEY) {
		++su16_Tcnt;
		if(++su16_TwinkleCnt >= 5) {			//�ڰ��Ű���ʱ����յ�ƽ�����
			su16_TwinkleCnt = 0;
			MCUO_LED_RED = !MCUO_LED_RED;
			MCUO_LED_GREEN = !MCUO_LED_RED;
		}

		if(su16_Tcnt >= 10*10) {				//���߼�⣬�������10s��ֱ�����������Ƶ���
			LED_Command = LED_RESTART_MPU;
			su16_Tcnt = 0;						//��ԭ
			su16_TwinkleCnt = 0;
			MCUO_LED_RED = LED_CLOSE;
			MCUO_LED_GREEN = LED_CLOSE;
		}
	}
	else {
		if(su16_Tcnt >= 2*10 && su16_Tcnt <= 5*10) {
			LED_Command = LED_EMERGENCY;
		}
		else if(su16_Tcnt >= 8*10) {
			LED_Command = LED_RESTART_MPU;
		}
		else {
			LED_Command = LED_STARTUP;
		}

		su16_Tcnt = 0;					//��ԭ
		su16_TwinkleCnt = 0;
		MCUO_LED_RED = LED_CLOSE;
		MCUO_LED_GREEN = LED_CLOSE;
	}
}


void LED_Emergency(void) {
	static UINT16 su16_Tcnt = 0;
	static UINT16 su16_TwinkleCnt = 0;
	
	if(!MCUI_EMERGENCY_KEY) {			//���ǰ��Ļ���������ȥѡ��
		su16_TwinkleCnt = 0;
		su16_Tcnt = 0;
		MCUO_LED_RED = 0;
		MCUO_LED_GREEN = 0;
		MCUO_BUZZER = 0;
		LED_Command = LED_RESCUE_PIVOT;
		return;
	}
	
	if(++su16_Tcnt < 40*10) {
		gu8_MosOpenFlag = 1;			//ǿ�ƴ�MOS
		if(++su16_TwinkleCnt >= 5) {	//�̵���˸40s
			su16_TwinkleCnt = 0;
			MCUO_LED_GREEN = !MCUO_LED_GREEN;
		}
	}
	else {
		su16_TwinkleCnt = 0;			//��ԭ
		su16_Tcnt = 0;
		gu8_MosOpenFlag = 0;
		MCUO_LED_GREEN = 0;
		LED_Command = LED_STARTUP;		//���ǽ����ȥ��ģ����������һ�£�����һ���ǵ�Ƭ������
	}
}


void LED_ReStartMPU(void) {
	MCU_RESET();						//��Ƭ������
}


void InitLED(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	//PA12_MCUO_BUZZER
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	//PC14_LED_RED��PC15_LED_GREEN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	//PA0_MCUI_KEY
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	LED_Command = LED_STARTUP;
	Led_Mode.all = 0;

	//MCUO_LED_GREEN = 1;
}


void APP_LED(void) {
	if(0 == g_st_SysTimeFlag.bits.b1Sys100msFlag) {
		return;
	}

	if(SystemStatus.bits.b1StartUpBMS) {
		return;
	}

	switch(LED_Command)	{
		case LED_STARTUP:
			LED_StartUp();
			break;

		case LED_FAULT_MONITOR:
			LED_LoadErr();			//���ȼ��纯��˳����ʾ
			LED_BatteryErr();
			LED_SocLow();
			break;

		case LED_RESCUE_PIVOT:
			LED_Rescue_Pivot();
			break;

		case LED_EMERGENCY:
			LED_Emergency();
			break;

		case LED_RESTART_MPU:
			LED_ReStartMPU();
			break;

		//���д���������ͬʱ���ֶ��ִ�������ң����������ȼ������⣬Ҫ�м�
		//��������д���õ�
		/*
		case LED_MONITOR:
			LED_SocLow();
			LED_BatteryErr();
			LED_LoadErr();
			break;
		*/
		default:
			break;
	}
}

