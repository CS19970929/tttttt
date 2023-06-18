#include "main.h"

volatile union SYS_TIME g_st_SysTimeFlag;

struct CBC_ELEMENT CBC_Element;

static INT8 fac_us = 0;	 // us
static INT16 fac_ms = 0; // ms

UINT8 g_u81msCnt = 0;
UINT8 g_u810msClockCnt = 0;
UINT8 g_u81msClockCnt = 0;

UINT8 gu8_200msCnt = 0;
UINT8 gu8_200msAccClock_Flag = 0;

void IWDG_HaltConfig(void);

void InitDelay(void)
{
	SysTick->CTRL &= ~(1 << 2); // ʹ���ⲿʱ��
	// ��仰����ʲô������������������ҵ�ϵͳʱ������ʮ����
	// fac_us = SystemCoreClock / 8000000;		//SysTickʱ����SYSCLK 8��Ƶ����SysTickʱ��Ƶ��=SYSCLK/8��1usҪ�Ƶĸ���Ϊ����/1MHz
	// fac_us = 6;
	// fac_us = (UINT8)(SystemCoreClock / 8000000);	//���ָĹ����ˣ����ǻ��ǲ���

	switch (SystemCoreClock)
	{
	case 8000000:
		fac_us = 1;
		fac_ms = (INT16)fac_us * 1000; // ÿ��ms��Ҫ��systickʱ����
		break;
	case 12000000:
		// fac_us = 1.5;
		fac_us = 1; // us��ò�׼�ˣ�12M����ֻ�ܱ�Ƶ
		fac_ms = (INT16)fac_us * 1500;
		break;
	case 48000000:
		fac_us = 6;
		fac_ms = (INT16)fac_us * 1000; // ÿ��ms��Ҫ��systickʱ����
		break;
	default:
		break;
	}
}

void __delay_us(UINT32 nus)
{
	UINT32 temp;
	SysTick->LOAD = nus * fac_us;			  // ʱ�����
	SysTick->VAL = 0x00;					  // ��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while ((temp & 0x01) && !(temp & (1 << 16))); // �ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;		// �رռ�����
	SysTick->VAL = 0X00;							// ��ռ�����
}

// ����Ƿ��жϷ�ʽ����ʱ������ʹ���ж�ʽ��ʱ�����ж���ʹ����ʱ������ж�Ƕ�����⣬�����׳���
void __delay_ms(UINT16 ms)
{
	UINT32 temp;
	SysTick->LOAD = (UINT32)ms * fac_ms;	  // ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL = 0x00;					  // ��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16))); // �ȴ�ʱ�䵽��

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // �رռ�����
	SysTick->VAL = 0X00;					   // ��ռ�����
}

void InitIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // ����GPIOA������ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // ����GPIOB������ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE); // ����GPIOC������ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE); // ����GPIOB������ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE); // ����GPIOB������ʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE); // ����GPIOF������ʱ��

	// PB2_LED1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PF7_WAKEUP_AFE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	// MCUO_DEBUG_LED1 = 1;
}

void InitTimer(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);		//ʱ��3ʹ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);

	// ��ʱ����ʼ��
	TIM_TimeBaseStructure.TIM_Period = 500 - 1;							 // ��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1; // ����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ����������Ƶ
	// TIM_TimeBaseStructure.TIM_Prescaler = 1; 					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ����������Ƶ
	// TIM_TimeBaseStructure.TIM_Prescaler = 16;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// ����ʱ�ӷָ�:TDTS = Tck_tim����ʱ�ӷ�Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM17, &TIM_TimeBaseStructure);			// ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(TIM17, TIM_IT_Update, ENABLE);					// ʹ��ָ���ж�,��������ж�

	/*�ж�Ƕ�����*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0; // ��ռ���ȼ�0����û��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM17, ENABLE); // ʹ��TIMx
}

// ʹ��LSI��38KHz
void Init_IWDG(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // ʹ��PWR����ʱ�ӣ�����ģʽ��RTC�����Ź�
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);		// �򿪶������Ź��Ĵ�������Ȩ��
	IWDG_SetPrescaler(IWDG_Prescaler_64);				// Ԥ��Ƶϵ��
	IWDG_SetReload(160);								// �������ؼ���ֵ��k = Xms / (1 / (40KHz/64)) = X/64*40; 4096���
						 // 800����1.28s��80����128ms
	IWDG_ReloadCounter(); // ι��
	IWDG_Enable();		  // ʹ��IWDG
}

void App_SysTime(void)
{
	static UINT8 s_u8Cnt1ms = 0;

	static UINT8 s_u8Cnt10ms = 0;
	// static UINT8 s_u8Cnt20ms = 0;
	static UINT8 s_u8Cnt50ms = 0;
	static UINT8 s_u8Cnt100ms = 0;

	static UINT8 s_u8Cnt200ms1 = 0;
	static UINT8 s_u8Cnt200ms2 = 4;
	static UINT8 s_u8Cnt200ms3 = 8;
	static UINT8 s_u8Cnt200ms4 = 12;
	static UINT8 s_u8Cnt200ms5 = 16;

	static UINT8 s_u8Cnt1000ms1 = 0;
	static UINT8 s_u8Cnt1000ms2 = 33;
	static UINT8 s_u8Cnt1000ms3 = 66;

	g_st_SysTimeFlag.bits.b1Sys1msFlag = 0;
	if (s_u8Cnt1ms != g_u81msClockCnt)
	{ // 1ms��ʱ��־
		s_u8Cnt1ms = g_u81msClockCnt;
		g_st_SysTimeFlag.bits.b1Sys1msFlag = 1;
	}

	g_st_SysTimeFlag.bits.b1Sys10msFlag1 = 0;
	g_st_SysTimeFlag.bits.b1Sys10msFlag2 = 0;
	g_st_SysTimeFlag.bits.b1Sys10msFlag3 = 0;
	g_st_SysTimeFlag.bits.b1Sys10msFlag4 = 0;
	g_st_SysTimeFlag.bits.b1Sys10msFlag5 = 0;
	if (s_u8Cnt10ms != g_u810msClockCnt)
	{ // 10ms��ʱ��־
		s_u8Cnt10ms = g_u810msClockCnt;
		switch (g_u810msClockCnt)
		{
		case 0:
			g_st_SysTimeFlag.bits.b1Sys10msFlag1 = 1;
			break;

		case 1:
			s_u8Cnt50ms++;
			g_st_SysTimeFlag.bits.b1Sys10msFlag2 = 1;
			break;

		case 2:
			s_u8Cnt100ms++;
			g_st_SysTimeFlag.bits.b1Sys10msFlag3 = 1;
			break;

		case 3:
			s_u8Cnt200ms1++; // ������һ��������һ��ѭ��Ȼ����λ��������BUG������
			s_u8Cnt200ms2++; // ���������һ��10ms������ı��־λ���䲻�ٽ���
			s_u8Cnt200ms3++;
			s_u8Cnt200ms4++;
			s_u8Cnt200ms5++;
			g_st_SysTimeFlag.bits.b1Sys10msFlag4 = 1;
			break;

		case 4:
			s_u8Cnt1000ms1++;
			s_u8Cnt1000ms2++;
			s_u8Cnt1000ms3++;
			g_st_SysTimeFlag.bits.b1Sys10msFlag5 = 1;
			break;

		default:
			break;
		}
	}

	g_st_SysTimeFlag.bits.b1Sys50msFlag = 0;
	if (s_u8Cnt50ms >= 5)
	{
		s_u8Cnt50ms = 0;
		g_st_SysTimeFlag.bits.b1Sys50msFlag = 1; // 50ms��ʱ��־
	}

	g_st_SysTimeFlag.bits.b1Sys100msFlag = 0;
	if (s_u8Cnt100ms >= 10)
	{
		s_u8Cnt100ms = 0;
		g_st_SysTimeFlag.bits.b1Sys100msFlag = 1; // 100ms��ʱ��־
	}

	g_st_SysTimeFlag.bits.b1Sys200msFlag1 = 0;
	g_st_SysTimeFlag.bits.b1Sys200msFlag2 = 0;
	g_st_SysTimeFlag.bits.b1Sys200msFlag3 = 0;
	g_st_SysTimeFlag.bits.b1Sys200msFlag4 = 0;
	g_st_SysTimeFlag.bits.b1Sys200msFlag5 = 0;
	if (s_u8Cnt200ms1 >= 20)
	{
		s_u8Cnt200ms1 = 0;
		g_st_SysTimeFlag.bits.b1Sys200msFlag1 = 1; // 200ms��ʱ��־
	}
	if (s_u8Cnt200ms2 >= 20)
	{
		s_u8Cnt200ms2 = 0;
		g_st_SysTimeFlag.bits.b1Sys200msFlag2 = 1; // 200ms��ʱ��־
	}
	if (s_u8Cnt200ms3 >= 20)
	{
		s_u8Cnt200ms3 = 0;
		g_st_SysTimeFlag.bits.b1Sys200msFlag3 = 1; // 200ms��ʱ��־
	}
	if (s_u8Cnt200ms4 >= 20)
	{
		s_u8Cnt200ms4 = 0;
		g_st_SysTimeFlag.bits.b1Sys200msFlag4 = 1; // 200ms��ʱ��־
#if 0
		if(!MCUI_SLEEP_DET) {
			MCUO_DEBUG_LED1 = !MCUO_DEBUG_LED1;
		}
#endif
		MCUO_DEBUG_LED1 = !MCUO_DEBUG_LED1;
	}
	if (s_u8Cnt200ms5 >= 20)
	{
		s_u8Cnt200ms5 = 0;
		g_st_SysTimeFlag.bits.b1Sys200msFlag5 = 1; // 200ms��ʱ��־
	}

	g_st_SysTimeFlag.bits.b1Sys1000msFlag1 = 0;
	g_st_SysTimeFlag.bits.b1Sys1000msFlag2 = 0;
	g_st_SysTimeFlag.bits.b1Sys1000msFlag3 = 0;
	if (s_u8Cnt1000ms1 >= 100)
	{
		s_u8Cnt1000ms1 = 0;
		g_st_SysTimeFlag.bits.b1Sys1000msFlag1 = 1; // 1000ms��ʱ��־
	}
	if (s_u8Cnt1000ms2 >= 100)
	{
		s_u8Cnt1000ms2 = 0;
		g_st_SysTimeFlag.bits.b1Sys1000msFlag2 = 1; // 1000ms��ʱ��־
	}
	if (s_u8Cnt1000ms3 >= 100)
	{
		s_u8Cnt1000ms3 = 0;
		g_st_SysTimeFlag.bits.b1Sys1000msFlag3 = 1; // 1000ms��ʱ��־
		// MCUO_DEBUG_LED2 = !MCUO_DEBUG_LED2;
	}
}

void TIM17_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM17, TIM_IT_Update) != RESET)
	{												 // ���TIM3�����жϷ������
		TIM_ClearITPendingBit(TIM17, TIM_IT_Update); // ���TIMx�����жϱ�־
		if ((++g_u81msCnt) >= 2)
		{ // 1ms
			g_u81msCnt = 0;
			g_u81msClockCnt++;

			if (g_u81msClockCnt >= 2)
			{ // 2ms
				g_u81msClockCnt = 0;
				g_u810msClockCnt++;
				if (g_u810msClockCnt >= 5)
				{ // 10ms
					g_u810msClockCnt = 0;
				}
			}
			if (++gu8_200msCnt >= 200)
			{
				gu8_200msCnt = 0;
				gu8_200msAccClock_Flag = 1;
			}
		}
	}
}
