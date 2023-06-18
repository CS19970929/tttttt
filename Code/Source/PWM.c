#include "main.h"

UINT32 DACx_OC_REF = 0;

UINT16 u16PeriodCH1 = 0;
UINT16 u16PeriodCH2 = 0;
UINT16 u16PeriodCH3 = 0;
UINT16 u16PeriodCH4 = 0;



void InitPWM_TIME3(void) {
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	//һ����ʼ����Ҫ������
	//PA6_PWM_OC_DSG��PA7_PWM-OC-REF
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	//ѡ��Ҫ�õ�GPIO���� 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//��仰������޷��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1);


	//������ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1; 					//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000000 - 1; 	//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ����������Ƶ	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  /* Time ��ʱ����Ϊ�����ؼ���ģʽ*/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	//��������PWM����ͨ��������ͨ����������Ĭ�ϲ������
	//CH1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				   	//����Ϊpwm1���ģʽ
	TIM_OCInitStructure.TIM_Pulse = u16PeriodCH1;								//����ռ�ձ�ʱ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;			//�����������
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;		//���������״̬
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	   	//ʹ�ܸ�ͨ�����
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  				//ʹ��TIM1��CCR1�ϵ�Ԥװ�ؼĴ�����TIM_SetCompare1()��������

	//CH2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 				   	//����Ϊpwm1���ģʽ
	TIM_OCInitStructure.TIM_Pulse = u16PeriodCH2;								//����ռ�ձ�ʱ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;			//�����������
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;		//���������״̬
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 	   	//ʹ�ܸ�ͨ�����
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  				//ʹ��TIM1��CCR1�ϵ�Ԥװ�ؼĴ�����TIM_SetCompare1()��������


	/* TIMX ������ʹ��*/
	TIM_Cmd(TIM3, ENABLE);
	  
	/* TIMX �����ʹ�� */
	//��仰��Ҫ��
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
}


void InitPWM(void) {
	UINT32 u32_DACxTemp;
	UINT32 u32_CS_Res;

	u32_DACxTemp = ((UINT32)OtherElement.u16CS_Cur_DSGmax*500)/(OtherElement.u16CS_Cur_CHGmax + OtherElement.u16CS_Cur_DSGmax);
	DACx_OC_REF = (u32_DACxTemp > 330 ? 4095 : (u32_DACxTemp<<12)/330) + (0<<16);		//ͨ��1��ͨ��2

	u32_CS_Res = ((UINT32)OtherElement.u16Sys_CS_Res*1000)/OtherElement.u16Sys_CS_Res_Num;		//����1000��
	//PWM-OC-DSG���Ƚϵ�ѹ���ȽϷ�ֵ��DSG�����Ǽ���
	u16PeriodCH1 = (DACx_OC_REF*662>>12) - (UINT32)OtherElement.u16CBC_Cur_DSG*u32_CS_Res*4/10/1000;

	//DAÇ����ѹ��ΪPWM�����PWM-OC-REF
	u16PeriodCH2 = DACx_OC_REF*1000/4096;

	InitPWM_TIME3();
}


//���ڲ���DMA�����������ֵ�޸ģ���Ҫ�����������
void App_PWM(void) {
	UINT32 u32_DACxTemp;
	UINT32 u32_CS_Res;

	u32_DACxTemp = ((UINT32)OtherElement.u16CS_Cur_DSGmax*500)/(OtherElement.u16CS_Cur_CHGmax + OtherElement.u16CS_Cur_DSGmax);
	DACx_OC_REF = (u32_DACxTemp > 330 ? 4095 : (u32_DACxTemp<<12)/330) + (0<<16);		//ͨ��1��ͨ��2

	u32_CS_Res = ((UINT32)OtherElement.u16Sys_CS_Res*1000)/OtherElement.u16Sys_CS_Res_Num;		//����1000��
	//PWM-OC-DSG���Ƚϵ�ѹ���ȽϷ�ֵ
	u16PeriodCH1 = (DACx_OC_REF*662>>12) - (UINT32)OtherElement.u16CBC_Cur_DSG*u32_CS_Res*4/10/1000;

	//DAÇ����ѹ��ΪPWM�����PWM-OC-REF
	u16PeriodCH2 = DACx_OC_REF*1000/4096;


	//TIM_SetAutoreload(TIM4, 1000);				//�����е�����Ҫ�ı�PWMƵ�ʵ���
 	TIM_SetCompare1(TIM3, u16PeriodCH1);		//�����е�����Ҫ�ı�PWMռ�ձȵ���
 	TIM_SetCompare2(TIM3, u16PeriodCH2);		//�����е�����Ҫ�ı�PWMռ�ձȵ���
}

