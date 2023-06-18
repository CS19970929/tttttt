#include "main.h"

struct RTC_ELEMENT RTC_time;

RTC_InitTypeDef RTC_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_DateTypeDef RTC_DateStructure;
RTC_AlarmTypeDef RTC_AlarmStructure;

UINT8 RTC_ClockConfig(void)
{
	__IO UINT16 StartUpCounter = 0, HSEStatus = 0;
	UINT8 result = 0;
	
	PWR_BackupAccessCmd(ENABLE); // �������RTC
	RCC_LSEConfig(RCC_LSE_ON);	 // ʹ���ⲿLSE����RCC_LSE_Bypass��·����˼Ӧ����ʹ�����LSEʱ�ӣ����ǵ�Ƭ�����ã���Χ��·��?
	do
	{
		HSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSERDY);
		StartUpCounter++;
	} while ((HSEStatus == RESET) && (StartUpCounter < LSE_START_TIMEOUT)); // �ȴ��� LSE Ԥ��

	if (StartUpCounter < LSE_START_TIMEOUT)
	{
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); // ��RTC ʱ��Դ����ΪLSE
		RCC_RTCCLKCmd(ENABLE);					// ʹ��RTCʱ��
		RTC_WaitForSynchro();					// �ȴ� RTC APB �Ĵ���ͬ��
		// ��Ƶ���ã�Flsi/((AsynchPrediv+1)(SynchPrediv+1)) = 1Hz = 1s
		RTC_InitStructure.RTC_AsynchPrediv = 99; // �첽��Ƶ��0x7F����������Խ��͹���
		RTC_InitStructure.RTC_SynchPrediv = 327; // ͬ����Ƶ��LSE = 32768Hz
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		result = 0;
	}
	else
	{
		// System_ERROR_UserCallback(ERROR_LSE);					//RTC������ΪLSE����
		RCC_LSICmd(ENABLE); // ʹ�� LSI ��
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
			; // �ȴ��� LSI Ԥ��

		RCC->BDCR = 0x10000;		 // ����Ǹ�����ߴ���ģ�ԭ���ǿ�����ѡ����LSE
		PWR_BackupAccessCmd(ENABLE); // ����û�����������ܻ��ѣ����Ա����RTC��λһ��
		RCC->BDCR = 0x8200;			 // �ٴ�������ʱ��Դ��û����

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);	 // ��RTC ʱ��Դ����ΪLSI
		RCC_RTCCLKCmd(ENABLE);					 // ʹ��RTCʱ��
		RTC_WaitForSynchro();					 // �ȴ� RTC APB �Ĵ���ͬ��
		RTC_InitStructure.RTC_AsynchPrediv = 99; // �첽��Ƶ��0x7F����������Խ��͹���
		RTC_InitStructure.RTC_SynchPrediv = 399; // ͬ����Ƶ��LSI = 40KHz
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		result = 1;
	}

	if (RTC_Init(&RTC_InitStructure) == ERROR)
	{
		result = 1;
	}
	return result;
}

UINT8 RTC_TimeConfig(void)
{
	UINT8 result = 0;

	RTC_DateStructure.RTC_Year = RTC_time.RTC_Time_Year;
	RTC_DateStructure.RTC_Month = RTC_time.RTC_Time_Month;
	RTC_DateStructure.RTC_Date = RTC_time.RTC_Time_Day;
	RTC_TimeStructure.RTC_H12 = RTC_H12_AM; // 24Сʱ��
	RTC_TimeStructure.RTC_Hours = RTC_time.RTC_Time_Hour;
	RTC_TimeStructure.RTC_Minutes = RTC_time.RTC_Time_Minute;
	RTC_TimeStructure.RTC_Seconds = RTC_time.RTC_Time_Second;

	if (RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == ERROR ||
		RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure) == ERROR)
	{
		// printf("\n\r>> !! RTC Set Time failed. !! <<\n\r");
		result = 1;
	}
	return result;
}

// ��û���ã���֪��Ч����Σ��жϾ�û�㶨
void RTC_AlarmConfig(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);					   // Disable the Alarm A
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_AM; // 24Сʱ��

#if 0
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = RTC_time.RTC_Alarm_Hour + (OtherElement.u16Sleep_RTC_WakeUpTime/60);
	UPDNLMT16(RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours,23,0);
	
	//�����Ǹ����ᳬ��59
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = RTC_time.RTC_Alarm_Minute + (OtherElement.u16Sleep_RTC_WakeUpTime%60);
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = RTC_time.RTC_Alarm_Second;
#endif

#if 1
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = RTC_time.RTC_Alarm_Hour + 0;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = RTC_time.RTC_Alarm_Minute + 0;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = RTC_time.RTC_Alarm_Second + 30;
#endif

	// Set the Alarm A
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 31;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date; // ѡ��0-31�죬����������
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;			   // 0-31���Alarm A ��Ӱ��
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);			   // ��������⣬��datasheetӦ����BCD��ʽ�����ǿ⺯��ҪBIN���ܱ�ΪBCD
																			   // RTC_Alarm_A����Ҫ�в��У���֪����ʲô��
	// RTC_AlarmShow();
	RTC_ITConfig(RTC_IT_ALRA, ENABLE); // Enable the RTC Alarm A Interrupt
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE); // Enable the alarm	A

	EXTI_ClearITPendingBit(EXTI_Line17); // �ⲿ�������жϣ��ܴ�standbyģʽ����
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn; // Enable the RTC Alarm Interrupt
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Init_RTC_Clock(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // ʹ��PWR����ʱ�ӣ�����ģʽ��RTC�����Ź�
	RTC_ClockConfig();									// RTCʱ������

	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != BKP_VALUE)
	{ // ��ȡ���������ֵ�Ƿ�д����
		if (0 == RTC_TimeConfig())
		{
			RTC_WriteBackupRegister(RTC_BKP_DR0, BKP_VALUE);
		}
		else
		{
			// ����ʧ�ܲ���
		}
		RTC_AlarmConfig();
	}
	else
	{ // ������λ���Ҫ��
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
		{ // ����ɶ
		  //("\r\n Power On Reset occurred....\n\r");
		  //++RTC_Faultcnt;
		}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
		{ // ����ɶ
		  // printf("\r\n External Reset occurred....\n\r");
		}
		RTC_ClearFlag(RTC_FLAG_ALRAF); // Clear the RTC Alarm Flag
		EXTI_ClearITPendingBit(EXTI_Line17);
		// RTC_TimeShow();				//Display the RTC Time and Alarm�����������õ�
		// RTC_AlarmShow();
	}
}

void Init_RTC_HaltWakeUp(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // ʹ��PWR����ʱ�ӣ�����ģʽ��RTC�����Ź�
	RTC_ClockConfig();									// RTCʱ������
}

void Init_RTC(void)
{
	UINT8 i;
	Init_RTC_HaltWakeUp();

#ifdef _HALT_ACTIVE_MODE
	Init_RTC_HaltWakeUp();
#else
// Init_RTC_Clock();
#endif

	// Ĭ��Ϊ0
	for (i = 0; i < 12; i++)
	{
		*(&RTC_time.RTC_Time_Year + i) = 0;
	}
}

void App_RTC(void)
{
#ifndef _HALT_ACTIVE_MODE
	static UINT8 u8RTCcnt = 0;
	if (0 == g_st_SysTimeFlag.bits.b1Sys200msFlag1)
	{
		return;
	}
	if (++u8RTCcnt < 5)
	{
		return;
	}
	u8RTCcnt = 0;
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
#endif
}

void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_ALRA) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALRA);
	}
	EXTI_ClearITPendingBit(EXTI_Line17);

	if (FlashReadOneHalfWord(FLASH_ADDR_WAKE_TYPE) != FLASH_VALUE_WAKE_RTC)
	{
		FlashWriteOneHalfWord(FLASH_ADDR_WAKE_TYPE, FLASH_VALUE_WAKE_RTC);
	}
}
