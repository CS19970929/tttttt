#include "main.h"

UINT8 SeriesNum = 16;

// ��ͬ����ά���ı��
const unsigned char SeriesSelect_AFE1[16][16] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 1��
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 2��
	{0, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 3   76920
	{0, 1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 4   76920
	{0, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 5   76920
	//{0 ,1 ,2 ,3 ,4 ,15,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0, 0},   	//6   76920 + AD	//��6��ӳ�䵽16��������˵�Ĺ�Ȼ����
	{0, 1, 4, 5, 6, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	   // 6   76930 		//����˵��û���ˣ���Ϊ930����������
	{0, 1, 2, 4, 5, 6, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0},	   // 7   76930
	{0, 1, 2, 4, 5, 6, 7, 9, 0, 0, 0, 0, 0, 0, 0, 0},	   // 8   76930
	{0, 1, 2, 3, 4, 5, 6, 7, 9, 0, 0, 0, 0, 0, 0, 0},	   // 9   76930
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0},	   // 10  76930
	{0, 1, 3, 4, 5, 6, 7, 9, 10, 11, 14, 0, 0, 0, 0, 0},   // 11  76940
	{0, 1, 2, 4, 5, 6, 7, 9, 10, 11, 12, 14, 0, 0, 0, 0},  // 12  76940
	{0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 14, 0, 0, 0},  // 13  76940
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 0, 0},  // 14  76940
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0}, // 15  76940
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15} // 16  76940 + AD	//����汾������16����
};

void InitVar(void);
void InitDevice(void);
void InitSci(void);
void App_Sci(void);
void MCU_ClockTest(void);

int main(void)
{
	InitDevice(); // ��ʼ�����裬������������λ����Ҫ����һ�£����ڻ���ȥ��
	InitVar();	  // ��ʼ������

	while (1)
	{
#if (defined _DEBUG_CODE)
		App_SysTime();
		// App_NormalSleepTest();
		Feed_IWatchDog;

#else
		App_SysTime();
		App_Sci();
		App_AFEGet();
		App_BQ769X0_Monitor();
		App_WarnCtrl();
		App_MOS_Relay_Ctrl();
		App_AnlogCal();
		App_E2promDeal();
		// App_RTC();
		App_CellBalance();
		App_SOC();
		App_SleepDeal(); // ����App_MOS_Relay_Control()����
		// App_Heat_Cool_Ctrl();
		App_ChargerLoad_Det();

		App_FlashUpdateDet();
		App_LogRecord();
		App_ProID_Deal();

		Feed_IWatchDog;
#endif
	}
}

// �����ʼ�����������׳�����
void InitDevice(void)
{
	SystemInit(); // ֱ�ӵ��þͿ����ˡ�
				  // A����reset�������ã�ʹ��HSI(8MHz)���С�resetĬ����ʹ��HSI���С�
				  // B������SetSysClock()��Ĭ��ʹ��8MHz�ⲿ����Ȼ������Ƶ�������Ƶ������ܳ���48MHz(��ʹ��12MHz�����Ը�Ϊ4��Ƶ)
				  // C�������Ƶʧ�ܣ����и�else������Ҹģ����һЩ��־λ����Ŀǰû��
				  // D�����Ӵ�����ֹͣģʽ���ػ�����ϵͳʱ�ӵ�HSE ������������ʱ����λ��Ӳ����������HSI ������
				  // E������֮�⣬�������ģʽҪ���ⲿ���񣬻���������HSI���У�Ȼ�����ⲿ����ͱ�Ƶ��
				  // F������һ���л�ʱ�ӵĺ�����SystemCoreClockUpdate()��ʹ�����������˽⡣
				  // G���ⲿ�����޸ĵĻ�������ͷ�ļ�HSE_VALUE��ֵ����Ӱ�촮�ڲ����ʡ�
				  // H�������ʹ��HSE��ֱ�Ӻ����ⲿ�����ɣ�ϵͳ��Ĭ�Ϸ���HSI��SystemCoreClock�Զ���Ϊ8M�������۲촮�ڲ����ʺ�I2CƵ���Ƿ��������
	Init_IAPAPP();

#if (defined _DEBUG_CODE)
	InitIO();
	InitDelay();
	InitTimer();
	InitSystemWakeUp();
	// MCU_ClockTest();

	// Init_IWDG();

#else
	IsSleepStartUp();
	InitIO();
	InitDelay();
	//__delay_ms(1000);
	InitTimer();
	InitSystemWakeUp();
	InitE2PROM(); // �ڲ�EEPROM������Ҫ��ʼ��
	InitSci();
	InitADC();

	InitData_SOC();
	Init_RTC(); // �������EEPROM�������ݺ��棡
				// �������LSE_32KHz�Ŀڣ���ʱ�ȹص�RTC�����������ʹIO������ʧЧ���ɿ�
	// Init_I2CSlaver();
	// InitHeat_Cool();
	InitMosRelay_DOx();
	Init_ChargerLoad_Det();

	// InitPWM();			//����CBC�����

	InitAFE1();

	Init_IWDG();
#endif
}

void InitVar(void)
{
	UINT16 i;

	// SystemMonitorResetData_EEPROM();							//��������ĳ�ʼ��Ĭ���������޸��ˣ�Ҫ�޸�EEPROM���ϵ��־λ
	InitSystemMonitorData_EEPROM();

	MCUO_RELAY_PRE = SystemStatus.bits.b1Status_Relay_PRE; // ѡ����Ҫ�������︳ֵ
	MCUO_RELAY_MAIN = SystemStatus.bits.b1Status_Relay_MAIN;

	// Switch����
	// Switch_OnOFF_Func.all = 0;
	// InitSwitchData_EEPROM();

	// ��ϵͳ������ϵͳ��ʼ��
	for (i = 0; i < ERROR_NUM; ++i)
	{
		//*(&System_ErrFlag.u8ErrFlag_Com_AFE1+i)  =  0;		//�в�������д�ͰѴ��������
	}

	// ������־λ��ʼ��
	g_stCellInfoReport.unMdlFault_First.all = 0;
	g_stCellInfoReport.unMdlFault_Second.all = 0;
	g_stCellInfoReport.unMdlFault_Third.all = 0;
	// ���α�����¼��ʼ��
	FaultPoint_First = 0;
	FaultPoint_Second = 0;
	FaultPoint_Third = 0;

	FaultPoint_First2 = 0;
	FaultPoint_Second2 = 0;
	FaultPoint_Third2 = 0;
	for (i = 0; i < Record_len; ++i)
	{
		Fault_record_First[i] = 0;
		Fault_record_Second[i] = 0;
		Fault_record_Third[i] = 0;

		Fault_record_First2[i] = 0;
		Fault_record_Second2[i] = 0;
		Fault_record_Third2[i] = 0;
	}
	Fault_Flag_Fisrt.all = 0;
	Fault_Flag_Second.all = 0;
	Fault_Flag_Third.all = 0;

	// �̵�������������ʼ��							//�������������
	// RelayCtrl_Command = RELAY_PRE_DET;	//�������������
	// HeatCtrl_Command = ST_HEAT_DET_SELF;
	// CoolCtrl_Command = ST_COOL_DET_SELF;

	// �������

	// ����д�Ͳ��ù�ǰ�浽�׶��������Ǹ�λ��(��EEPROM�ܶ���ط���)
	SeriesNum = OtherElement.u16Sys_SeriesNum;
	g_u32CS_Res_AFE = ((UINT32)OtherElement.u16Sys_CS_Res_Num * 844 << 10) / OtherElement.u16Sys_CS_Res / 100; // ��CS��������

	LogRecord_Flag.bits.Log_StartUp = 1;
}

void App_WakeUpAFE(void)
{
	MCUO_WAKEUP_AFE = 0;
	__delay_ms(1);
	MCUO_WAKEUP_AFE = 1;
	__delay_ms(5); // max 2ms��tBOOT_max
	MCUO_WAKEUP_AFE = 0;
	__delay_ms(10); // �Լ죬10ms��tBOOTREADY
}

void InitSystemWakeUp(void)
{
	// MCUO_SD_DRV_CHG = 0;

	// MCUO_PWSV_STB = 0;

	// MCUO_PWSV_LDO = 1;
	// MCUO_PWSV_CTR = 1;

	App_WakeUpAFE();
	MCUO_BEL_EN = 1;

	MCUO_AFE_ALARM = 1;

	// MCUO_PW_RS485_EN = 1;

	__delay_ms(20);
}

void InitSci(void)
{
	InitUSART_CommonUpper();
	InitUSART_UartClient();
}

void App_Sci(void)
{
	App_CommonUpper();
	App_UartClient_Updata();
}

void MCU_ClockTest(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_DBGMCU, ENABLE);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_0); // ���AFѡ����оƬ�ֲ��reg��

	RCC->CFGR |= RCC_CFGR_MCO_SYSCLK;
	// RCC->CFGR |= RCC_CFGR_MCO_HSE;
}
