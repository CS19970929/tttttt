#include "main.h"

enum BALANCE_STATE_E g_enBalanceState = BALANCE_ST_INIT;
enum CELL_BALANCE_STATUS_E g_enCellBalanceStatus[CELL_NUMS_MAX];
UINT8 g_u8CellBalanceFilterCnt[CELL_NUMS_MAX];
 
UINT16 CellBalFlag_AFE1;		//ʵ�ʴ�����־λ�洢�����ڲ���AFE�Ĵ���
UINT8  g_u8CBnMonitor;			//�Ƿ��е����Ҫ����ı�־λ

//UINT16 g_u16CBnFLAG_ToUpper;    //�ϴ�����λ����
UINT8  g_u8CBn_StatusFlag;		//���ڿ���Mos����Relay

 
void CB_ChangeUpperFlag(enum CELL_BALANCE_FLAG_UPPER type) {
	UINT16 temp1;
	UINT8 i;
	switch(type) {
		case CELL_BALANCE_COLSE:
			g_stCellInfoReport.u16BalanceFlag1 = 0;
			break;
		case CELL_BALANCE_ON_ODD:
			g_stCellInfoReport.u16BalanceFlag1 = 0;
			temp1 = CellBalFlag_AFE1 & ODD_SELECT;
			for(i = 0; i < SeriesNum; ++i) {	//�����ͬʱ����16��������Ҫ�������
				g_stCellInfoReport.u16BalanceFlag1 |= (((temp1>>SeriesSelect_AFE1[SeriesNum-1][i])&0x0001)<<i);
			}
			break;
			
		case CELL_BALANCE_ON_EVEN:
			g_stCellInfoReport.u16BalanceFlag1 = 0;
			temp1 = CellBalFlag_AFE1 & EVEN_SELECT;
			for(i = 0; i < SeriesNum; ++i) {
				g_stCellInfoReport.u16BalanceFlag1 |= (((temp1>>SeriesSelect_AFE1[SeriesNum-1][i])&0x0001)<<i);
			}
			break;
			
		default:
			g_stCellInfoReport.u16BalanceFlag1 = 0;
			break;
	}
}


void CB_StateCalculate(void) {
    UINT8 i;
	UINT8 ts_u8CBChanged = 0;
	UINT16 VC_min = g_stCellInfoReport.u16VCellMin;
	
	g_u8CBnMonitor = 0;
	for(i = 0; i < SeriesNum; i++) {
		if(CELL_BALANCE_STATUS_OFF == g_enCellBalanceStatus[i]) {
			if((g_stCellInfoReport.u16VCell[i] >= OtherElement.u16Balance_OpenVoltage)\
				&&(g_stCellInfoReport.u16VCell[i] >= OtherElement.u16Balance_OpenWindow + VC_min)) {
				if((++g_u8CellBalanceFilterCnt[i]) >= TIME_1000MS_2S) {
					g_enCellBalanceStatus[i] = CELL_BALANCE_STATUS_ON_VOLT_DELTA;
					g_u8CellBalanceFilterCnt[i] = 0;
					ts_u8CBChanged = 1;
				}
			}
			else {
				if(g_u8CellBalanceFilterCnt[i] > 0)
					--g_u8CellBalanceFilterCnt[i];
			}
		}
		else if(CELL_BALANCE_STATUS_ON_VOLT_DELTA == g_enCellBalanceStatus[i]) {
			if((g_stCellInfoReport.u16VCell[i] < OtherElement.u16Balance_OpenVoltage)\
				|| (g_stCellInfoReport.u16VCell[i] < OtherElement.u16Balance_CloseWindow + VC_min)) {
				if((++g_u8CellBalanceFilterCnt[i]) >= TIME_1000MS_2S) {
					g_enCellBalanceStatus[i] = CELL_BALANCE_STATUS_OFF;
					g_u8CellBalanceFilterCnt[i] = 0;
					ts_u8CBChanged = 1;
				}
			}			
			else {	
				if(g_u8CellBalanceFilterCnt[i] > 0)
					--g_u8CellBalanceFilterCnt[i];
			}
		}

		g_u8CBnMonitor += g_enCellBalanceStatus[i];
	}


	if(ts_u8CBChanged) {		//�б仯���޸�������־λ��û�仯���޸�������־λ
		CellBalFlag_AFE1 = 0;
		for(i = 0; i < SeriesNum; ++i) {		//������16������6��Ҳ���øģ�ӳ�䵽16��
			CellBalFlag_AFE1 |= ((UINT16)(g_enCellBalanceStatus[i] > 0 ? 1 : 0)<<SeriesSelect_AFE1[SeriesNum-1][i]);
		}
	}
}

UINT8 CB_AFERegistersCtrl(enum CELL_BALANCE_FLAG_UPPER CellBalance_Flag) {
	UINT8 result = 0;
	switch(CellBalance_Flag) {
		case CELL_BALANCE_COLSE:
			Registers_AFE1.CellBal1.CellBal1Byte = 0;
			Registers_AFE1.CellBal2.CellBal2Byte = 0;
			Registers_AFE1.CellBal3.CellBal3Byte = 0;
			result += I2CWriteBlockWithCRC(DEVICE_ADDR_AFE1, CELLBAL1, &(Registers_AFE1.CellBal1.CellBal1Byte), 3);
			break;
	
		case CELL_BALANCE_ON_ODD:
			Registers_AFE1.CellBal1.CellBal1Byte = (UINT8)(CellBalFlag_AFE1&ODD_SELECT&0x001F);
			Registers_AFE1.CellBal2.CellBal2Byte = (UINT8)(((CellBalFlag_AFE1&ODD_SELECT)>>5)&0x001F);	//��ѡͨ������λ����Ȼ�����߼�����-->6��1��ż����(��Ȼʵ��ûӰ��)
			Registers_AFE1.CellBal3.CellBal3Byte = (UINT8)(((CellBalFlag_AFE1&ODD_SELECT)>>10)&0x001F);
			result += I2CWriteBlockWithCRC(DEVICE_ADDR_AFE1, CELLBAL1, &(Registers_AFE1.CellBal1.CellBal1Byte), 3);
			break;
			
		case CELL_BALANCE_ON_EVEN:
			Registers_AFE1.CellBal1.CellBal1Byte = (UINT8)(CellBalFlag_AFE1&EVEN_SELECT&0x001F);
			Registers_AFE1.CellBal2.CellBal2Byte = (UINT8)(((CellBalFlag_AFE1&EVEN_SELECT)>>5)&0x001F);
			Registers_AFE1.CellBal3.CellBal3Byte = (UINT8)(((CellBalFlag_AFE1&EVEN_SELECT)>>10)&0x001F);
			result += I2CWriteBlockWithCRC(DEVICE_ADDR_AFE1, CELLBAL1, &(Registers_AFE1.CellBal1.CellBal1Byte), 3);
			break;
			
		default:
			Registers_AFE1.CellBal1.CellBal1Byte = 0;
			Registers_AFE1.CellBal2.CellBal2Byte = 0;
			Registers_AFE1.CellBal3.CellBal3Byte = 0;
			result += I2CWriteBlockWithCRC(DEVICE_ADDR_AFE1, CELLBAL1, &(Registers_AFE1.CellBal1.CellBal1Byte), 3);	
			break;
	}
	return result;

}


void CellBalance_DataInit(void) {
    UINT8 i;
	GPIO_InitTypeDef  GPIO_InitStructure;
	
    for(i = 0;i < CELL_NUMS_MAX;i++) {
        g_enCellBalanceStatus[i] = CELL_BALANCE_STATUS_OFF;
        g_u8CellBalanceFilterCnt[i] = 0;
    }
	CellBalFlag_AFE1 = 0;
	g_u8CBn_StatusFlag = 0;
	g_stCellInfoReport.u16BalanceFlag1 = 0;

    Registers_AFE1.CellBal1.CellBal1Byte = 0;
    Registers_AFE1.CellBal2.CellBal2Byte = 0;
    Registers_AFE1.CellBal3.CellBal3Byte = 0;
	I2CWriteBlockWithCRC(DEVICE_ADDR_AFE1, CELLBAL1, &(Registers_AFE1.CellBal1.CellBal1Byte), 3);
	
	if(SeriesNum%10 == 6) {
		//PB7_CB_6_16
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		MCUO_CB_6_16 = 0;
	}

	//if(SeriesNum == 16)MCUO_EXT_CB = 0;
	g_enBalanceState = BALANCE_ST_MONITOR;
}


void CellBalance_Monitor(UINT8 OnOFF_Ctrl) {
	static UINT8 s_u8BnRecord = 0;
	static UINT8 su8_Cur_Flag = 0;
	static UINT16 su16_Silence_Tcnt = 0;

	if((g_stCellInfoReport.u16Ichg > 10 || g_stCellInfoReport.u16IDischg > 10)	//���þ��⣬�е���������
		|| (g_stCellInfoReport.u16VCellMax < OtherElement.u16Balance_OpenVoltage)	//����ѹû����������ѹ
		|| (g_stCellInfoReport.u16VCellDelta < OtherElement.u16Balance_CloseWindow)	//ѹ����ڹرմ�������
		|| !OnOFF_Ctrl) {
		if(g_stCellInfoReport.u16BalanceFlag1 || CellBalFlag_AFE1 || g_u8CBn_StatusFlag) {
		  g_enBalanceState = BALANCE_ST_INIT;
		}
		if(g_stCellInfoReport.u16Ichg > 10 || g_stCellInfoReport.u16IDischg > 10) {
			su8_Cur_Flag = 1;
			if(su16_Silence_Tcnt)su16_Silence_Tcnt = 0;
		}	
	  	return;
	}

	//����5min
	if(su8_Cur_Flag) {
		if(++su16_Silence_Tcnt >= 1*60*5) {
			su16_Silence_Tcnt = 0;
			su8_Cur_Flag = 0;
		}
		else {
			return;
		}
	}

	CB_StateCalculate();		// change the CBnTIME reg when some cell balance stu changed
	if(g_u8CBnMonitor > 0) {
		g_enBalanceState = BALANCE_ST_ODD_ON;
		if(!s_u8BnRecord) {
			System_ERROR_UserCallback(ERROR_BALANCED);
			s_u8BnRecord = 1;
		}
	}
	else {
		g_enBalanceState = BALANCE_ST_OFF;
		if(s_u8BnRecord)s_u8BnRecord = !s_u8BnRecord;
	}
}


void CellBalance_StateOddOn(UINT8 OnOFF_Ctrl) {
	static UINT8 s_u8Select = 0;
	static UINT16 ts_u8TempCnt = 0;
	
	if(!OnOFF_Ctrl) {
		ts_u8TempCnt = 0;
		s_u8Select = 0;
		g_enBalanceState = BALANCE_ST_MONITOR;
		//BnElement.u16_RefreshData_Flag = 1;		//��Ҫˢ��������
	}
	
	switch(s_u8Select) {
		case 0:
			if(0 == (CellBalFlag_AFE1&ODD_SELECT)) {
				g_enBalanceState = BALANCE_ST_EVEN_ON;		//����Ҫ��������������ֱ������
				break;
			}
			if(!CB_AFERegistersCtrl(CELL_BALANCE_ON_ODD)) {
				CB_ChangeUpperFlag(CELL_BALANCE_ON_ODD);
				++s_u8Select;
				if(0 != Balance_OpenT_MOS) {
					g_u8CBn_StatusFlag = 1;			//�����Ż��㣬��������ż��һ��Ϊ0����������Դ�˷ѵ�BUG
				}
			}
			break;
			
		case 1:
			if((++ts_u8TempCnt) >= Balance_OpenT_ODD) {
				ts_u8TempCnt = 0;
        		g_enBalanceState = BALANCE_ST_EVEN_ON;
				s_u8Select = 0;
    		}
			break;
			
		default:
			s_u8Select = 0;
			break;
	}
}


void CellBalance_StateEvenOn(UINT8 OnOFF_Ctrl) {
	static UINT8 s_u8Select = 0;
	static UINT16 ts_u8TempCnt = 0;

	if(!OnOFF_Ctrl) {
		ts_u8TempCnt = 0;
		s_u8Select = 0;
		g_enBalanceState = BALANCE_ST_MONITOR;
		//BnElement.u16_RefreshData_Flag = 1;		//��Ҫˢ��������
	}
	
	switch(s_u8Select) {
		case 0:
			if(0 == (CellBalFlag_AFE1&EVEN_SELECT)) {
				g_enBalanceState = BALANCE_ST_OFF;		//����Ҫ��������������ֱ������
				break;
			}
			if(!CB_AFERegistersCtrl(CELL_BALANCE_ON_EVEN)) {
				CB_ChangeUpperFlag(CELL_BALANCE_ON_EVEN);
				++s_u8Select;
				if(0 != Balance_OpenT_MOS) {
					g_u8CBn_StatusFlag = 1;
				}
			}
			break;
			
		case 1:
			if((++ts_u8TempCnt) >= Balance_OpenT_EVEN) {
				ts_u8TempCnt = 0;
        		g_enBalanceState = BALANCE_ST_OFF;
				s_u8Select = 0;
    		}
			break;
			
		default:
			s_u8Select = 0;
			break;
	}
}


//�������Ӧ�ÿ�����
void CellBalance_StateOFF(UINT8 OnOFF_Ctrl) {
	static UINT8 s_u8Select = 0;
	static UINT16 ts_u8TempCnt = 0;

	if(!OnOFF_Ctrl) {
		ts_u8TempCnt = 0;
		s_u8Select = 0;
		g_enBalanceState = BALANCE_ST_MONITOR;
		//BnElement.u16_RefreshData_Flag = 1;		//��Ҫˢ��������
	}

	switch(s_u8Select) {
		case 0:
			if(0 == Balance_OpenT_MOS) {
				if(0 == g_u8CBnMonitor) {							//ѭ����������⵽��Ҫ����Źرա�
					if(!CB_AFERegistersCtrl(CELL_BALANCE_COLSE)) {
						CB_ChangeUpperFlag(CELL_BALANCE_COLSE);
					}
				}
				g_enBalanceState = BALANCE_ST_MONITOR;
			}
			else {
				if(!CB_AFERegistersCtrl(CELL_BALANCE_COLSE))  {		//������������ģ������������������һ���ٻ�ȥ
					CB_ChangeUpperFlag(CELL_BALANCE_COLSE);
					++s_u8Select;
					g_u8CBn_StatusFlag = 0;
				}
			}
			break;
			
		case 1:
			if((++ts_u8TempCnt) >= Balance_OpenT_MOS+OtherElement.u16Sys_PreChg_Time) {	//����Ԥ��ʱ��
				ts_u8TempCnt = 0;
	    		g_enBalanceState = BALANCE_ST_MONITOR;
				s_u8Select = 0;
			}
			break;
			
		default:
			s_u8Select = 0;
			break;
	}
}


void App_CellBalance(void) {
	if(STARTUP_CONT == System_FUNC_StartUp(SYSTEM_FUNC_STARTUP_BALANCE)) {
		return;
	}
	if(0 == g_st_SysTimeFlag.bits.b1Sys1000msFlag2) {
		return;
	}

    switch(g_enBalanceState) {
		case BALANCE_ST_INIT:
			CellBalance_DataInit();
			break;
        case BALANCE_ST_MONITOR:
            CellBalance_Monitor(System_OnOFF_Func.bits.b1OnOFF_Balance);
            break;
        case BALANCE_ST_ODD_ON:
            CellBalance_StateOddOn(System_OnOFF_Func.bits.b1OnOFF_Balance);
            break;
        case BALANCE_ST_EVEN_ON:
            CellBalance_StateEvenOn(System_OnOFF_Func.bits.b1OnOFF_Balance);
            break;
        case BALANCE_ST_OFF:
            CellBalance_StateOFF(System_OnOFF_Func.bits.b1OnOFF_Balance);
            break;			
        default:
			g_enBalanceState = BALANCE_ST_INIT;
            break;
    }
}


