#include "main.h"

enum RELAY_CTRL_STATUS RelayCtrl_Command = RELAY_PRE_DET;
enum MOS_CTRL_STATUS MOSCtrl_Command = MOS_PRE_DET;

volatile union Switch_OnOFF_Function Switch_OnOFF_Func;
UINT8 gu8_DsgFirstOpen_Flag = 0;

// ���ڸ�������
void RefreshData_Drivers(void)
{
	static UINT8 su8_OnOFF_Status = 0;

	// ��Ҫ����ϸ�ֵ�Ĳ���
	Driver_Element.Fault_Flag.all = g_stCellInfoReport.unMdlFault_Third.all;
	Driver_Element.u16_CurChg = g_stCellInfoReport.u16Ichg;
	Driver_Element.u16_CurDsg = g_stCellInfoReport.u16IDischg;

	// ��Ϣ������
	if (SystemStatus.bits.b1Status_BnCloseIO || SystemStatus.bits.b1Status_HeatCloseIO || SystemStatus.bits.b1Status_CBCCloseIO || System_ErrFlag.u8ErrFlag_Com_AFE1 || System_ErrFlag.u8ErrFlag_Com_AFE2 || System_ErrFlag.u8ErrFlag_Com_EEPROM || System_ErrFlag.u8ErrFlag_Store_EEPROM || CBC_Element.u8CBC_CHG_ErrFlag || CBC_Element.u8CBC_DSG_ErrFlag || System_ERROR_UserCallback(ERROR_STATUS_TEMP_BREAK) /*|| Sleep_Mode.all & 0x2F != 0*/)
	{

		Driver_Element.DriverForceExt.bits.b2_DriverOFF_Flag = FORCE_CLOSE_MODE; // CBC�����ŵ�����
	}
	else
	{
		Driver_Element.DriverForceExt.bits.b2_DriverOFF_Flag = FORCE_KEEP_MODE;
	}

	// ���д��������տ�ʼִ��һ�����������ִ���ˣ�ת����һ����������ִ��������ȴ����໥�л�
	switch (su8_OnOFF_Status)
	{
	case 0:
		if (Driver_Element.u8_FuncOFF_Flag)
		{
			System_OnOFF_Func.bits.b1OnOFF_MOS_Relay = 0;

			if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_OV)
			{ // �������ĸ������ˣ���������ʹ��
				// Sleep_Mode.bits.b1VcellOVP = 1;		//���������ɰ�(��ɽ�����޵�ǹ���߼�⹦��)ʹ����
				ChargerLoad_Func.bits.b1OFFDriver_Ovp = 1;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_UV)
			{
				// Sleep_Mode.bits.b1VcellUVP = 1;		//��������ʹ����
				ChargerLoad_Func.bits.b1OFFDriver_Uvp = 1;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Ichg)
			{
				ChargerLoad_Func.bits.b1OFFDriver_ChgOcp = 1;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Idsg)
			{
				ChargerLoad_Func.bits.b1OFFDriver_DsgOcp = 1;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Imain)
			{
				// ChargerLoad_Func.bits.b1OFFDriver_Ocp = 1;
				// �ѱ�ȡ��
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Vdelta)
			{
				ChargerLoad_Func.bits.b1OFFDriver_Vdelta = 1;
			}

			su8_OnOFF_Status = 1;
		}
		break;

	case 1:
		if (System_OnOFF_Func.bits.b1OnOFF_MOS_Relay)
		{
			Driver_Element.u8_FuncOFF_Flag = 0; // ��ԭ

			if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_OV)
			{ // �������ĸ������ˣ���������ʹ��
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_OV = 0;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_UV)
			{
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_UV = 0;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Ichg)
			{
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Ichg = 0;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Idsg)
			{
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Idsg = 0;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Imain)
			{
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Ocp_Imain = 0;
			}
			else if (Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Vdelta)
			{
				Driver_Element.MosRelay_Status.bits.b1_FuncOFF_Vdelta = 0;
			}

			su8_OnOFF_Status = 0;
		}
		break;

	default:
		break;
	}
}

// �����������д������
void GetData_Drivers(void)
{
	// SystemStatus.all = ((UINT32)(Driver_Element.MosRelay_Status.all&0x00FE))|(SystemStatus.all&0xFFFFFF01);
}

void InitData_Drivers(void)
{
	Driver_Element.u16_PreChg_Time = OtherElement.u16Sys_PreChg_Time;

	Driver_Element.u16_PreChg_Duty = 10;
	Driver_Element.u16_PreChg_Period = 1;

	// Driver_Element.u16_VirCur_Chg = OtherElement.u16Sleep_VirCur_Chg;
	// Driver_Element.u16_VirCur_Dsg = OtherElement.u16Sleep_VirCur_Dsg;

	// Ϊ�˴�����Ӵ򿪣��п�����������µ�طſյ�����(���Ӵ���������Ϊ����
	// MOS��Ԥ������(�Ӵ�������û���������)����Ϊд��2A������������2A���˳�Ԥ����ƣ����̴򿪷ŵ��
	Driver_Element.u16_VirCur_Chg = 0;
	Driver_Element.u16_VirCur_Dsg = 0;

	Driver_Element.u16_10msForceOpenT_Ovp = 3000; // Ĭ��30s
	Driver_Element.u16_10msForceOpenT_Uvp = 3000; // Ĭ��30s

	Driver_Element.u8_DriverCtrl_Right = 1; // AFE����
}

// ʹ�÷���
void App_DI1_Switch(void)
{
#ifdef _DI_SWITCH_DSG_ONOFF
	static UINT16 su16_AntiShake_Cnt1 = 0;

#if (defined _MOS)
	if ((1 == MCUI_ENI_DI1) && (g_stCellInfoReport.u16Ichg < 20))
	{
		if (++su16_AntiShake_Cnt1 >= 2)
		{
			su16_AntiShake_Cnt1 = 2;
			Driver_Element.DriverForceExt.bits.b2_Force_MOS_DSG = FORCE_CLOSE_MODE;
		}
	}
	else
	{
		if (su16_AntiShake_Cnt1)
		{
			--su16_AntiShake_Cnt1;
			return;
		}
		Driver_Element.DriverForceExt.bits.b2_Force_MOS_DSG = FORCE_KEEP_MODE;
	}
#else
	if (1 == MCUI_ENI_DI1)
	{
		if (++su16_AntiShake_Cnt1 >= 2)
		{
			su16_AntiShake_Cnt1 = 2;
			Driver_Element.DriverForceExt.bits.b2_Force_Relay_DSG = FORCE_CLOSE_MODE;
		}
	}
	else
	{
		if (su16_AntiShake_Cnt1)
		{
			--su16_AntiShake_Cnt1;
			return;
		}
		Driver_Element.DriverForceExt.bits.b2_Force_Relay_DSG = FORCE_KEEP_MODE;
	}
#endif
#endif

#ifdef _DI_SWITCH_SYS_ONOFF
	static UINT16 su16_AntiShake_Cnt1 = 0;

	if (1 == MCUI_ENI_DI1)
	{
		if (++su16_AntiShake_Cnt1 >= 2)
		{
			su16_AntiShake_Cnt1 = 2;
			Sleep_Mode.bits.b1ForceToSleep_L2 = 1;
		}
	}
	else
	{
		if (su16_AntiShake_Cnt1)
		{
			--su16_AntiShake_Cnt1;
			return;
		}
		// SleepElement.Sleep_Mode.bits.b1ForceToSleep_L2_L2 = 0;
	}
#endif

#ifdef _DI_SWITCH_longKEY_ONOFF
	static UINT16 su16_AntiShake_Cnt2 = 0;
	static uint8_t sleepflag = 0;

	if (0 == MCUI_ENI_DI1)
	{
		//++su16_AntiShake_Cnt2;
		if (++su16_AntiShake_Cnt2 >= 300)
		{
			su16_AntiShake_Cnt2 = 0;
			Sleep_Mode.bits.b1ForceToSleep_L2 = 1;

			sleepflag = 1;
		}
	}

#endif // _DI_SWITCH_longKEY_ONOFF
}

void Drivers_External_Ctrl(void)
{
	static UINT8 su8_Ctrl_Tcnt = 0;

	if (Driver_Element.u8_DriverCtrl_Right)
	{
		// 100ms����һ��
		if (++su8_Ctrl_Tcnt >= 10)
		{
			su8_Ctrl_Tcnt = 0;
			BQ769X0_DriverMos_Ctrl(GPIO_CHG, Driver_Element.MosRelay_Status.bits.b1Status_MOS_CHG);
			BQ769X0_DriverMos_Ctrl(GPIO_DSG, Driver_Element.MosRelay_Status.bits.b1Status_MOS_DSG);
		}
	}

	// MCUO_MOS_PRE = Driver_Element.MosRelay_Status.bits.b1Status_MOS_PRE;
}

void InitMosRelay_DOx(void)
{
	// �����������ţ����ⲻ��
	GPIO_InitTypeDef GPIO_InitStructure;

	// PA8_MCUO_RELAY_PRE��PA8_MCUO_MOS_PRE
	// InitDrivers_GPIO(GPIOA, GPIO_Pin_8, GPIO_PreCHG);

	// // PB13_MCUO_RELAY_CHG��PB13_MCUO_MOS_CHG
	// InitDrivers_GPIO(GPIOB, GPIO_Pin_13, GPIO_CHG);

	// // PB14_MCUO_RELAY_DSG��PB14_MCUO_MOS_DSG
	// InitDrivers_GPIO(GPIOB, GPIO_Pin_14, GPIO_DSG);

	// // PB14_MCUO_RELAY_MAIN��Լ���ŵ��Ϊ���Ӵ���
	// InitDrivers_GPIO(GPIOB, GPIO_Pin_14, GPIO_MAIN);

	// PC13_DI1_ENI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	InitData_Drivers();
}

void App_MOS_Relay_Ctrl(void)
{
#if defined(_MOS)
	if (STARTUP_CONT == System_FUNC_StartUp(SYSTEM_FUNC_STARTUP_MOS))
	{
		return;
	}
#elif defined(_RELAY) // ͬ�ڷֿ����⣬TODO
	if (STARTUP_CONT == System_FUNC_StartUp(SYSTEM_FUNC_STARTUP_RELAY))
	{
		return;
	}
#endif

	if (0 == g_st_SysTimeFlag.bits.b1Sys10msFlag1)
	{
		return;
	}

	App_DI1_Switch();
	RefreshData_Drivers();
	GetData_Drivers();
	Drivers_External_Ctrl();

#if (defined _RELAY_SAME_DOOR_NO_PRECHG)
	Drivers_RelaySameDoor_NoPreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _RELAY_SAME_DOOR_HAVE_PRECHG)
	Drivers_RelaySameDoor_HavePreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _RELAY_DIFF_DOOR_NO_PRECHG)
	Drivers_RelayDiffDoor_NoPreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _RELAY_DIFF_DOOR_HAVE_PRECHG)
	Drivers_RelayDiffDoor_HavePreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _MOS_SAME_DOOR_NO_PRECHG)
	Drivers_MosSameDoor_NoPreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _MOS_SAME_DOOR_HAVE_PRECHG)
	Drivers_MosSameDoor_HavePreChg(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#elif (defined _MOS_BOOTSTRAP_CIR)
	Drivers_MosBootStrap_Cir(g_st_SysTimeFlag.bits.b1Sys10msFlag1, System_OnOFF_Func.bits.b1OnOFF_MOS_Relay);
#endif

	/*
	if(SystemStatus.bits.b1Status_MOS_PRE) {
		MCUO_DEBUG_LED1 = 1;
	}
	*/
}
