#include "main.h"

__IO UINT16	g_u16ADCValFilter[ADC_NUM];		//���λ�����ܸ�

INT32 g_u32ADCValFilter2[ADC_NUM];          //ADC���ݻ���2���������ˣ�ԭ����UINT32���ڼ�����̳����ˣ�
INT32 g_i32ADCResult[ADC_NUM];             	//ADC�������

UINT16	g_u16IoutOffsetAD;
UINT16	g_u16Q7BusCurr_CHG;
UINT16	g_u16Q7BusCurr_DSG;

const UINT16 iSheldTemp_10K[LENGTH_TBLTEMP_PORT_10K] = 
{
    //AD		(Temp+40)*10
    3771	,	100	,	//-30
    3683	,	150	,	//-25
    3580	,	200	,	//-20
    3460	,	250	,	//-15
    3323	,	300	,	//-10
    3169	,	350	,	//-5
    3004	,	400	,	//0
    2820	,	450	,	//5
    2633	,	500	,	//10
    2437	,	550	,	//15
    2241	,	600	,	//20
    2048	,	650	,	//25
    1859	,	700	,	//30
    1679    ,	750	,	//35
    1509    ,	800	,	//40
    1351    ,	850	,	//45
    1204    ,	900	,	//50
    1073 	,	950	,	//55
    953 	,	1000,	//60
    845 	,	1050,	//65
    749 	,	1100,	//70
    664 	,	1150,	//75
    588 	,	1200,	//80
    522 	,	1250,	//85
    463 	,	1300,	//90
    411 	,	1350,	//95
    366 	,	1400,	//100
    326 	,	1450,	//105
};




/*
��������
1��ͨ�� uint16_t ConvData[8]����DMA���˵�ADCת����ֵ���������������ֵ��˳�����Ǻ�ADC����˳���Ӧ�ġ�
2��������7��ͨ����ADC��������ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Backward��
   �Ƕ�Ӧ˳���ǣ�0->0,1->7,2->6��7->1 �� 
3��������ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward��
   �Ƕ�Ӧ˳���ǣ�0->7,1->0,2->1��7->6 ������ԭ��F0��ADC��ʹ��֮ǰ��ҪУ׼��
4�����7λ��У׼ֵҲ�Ƿ���ADC_DR�еģ���Ҳ�ᴥ��DMA����
5�����Բ���F0��ADC-DMA���̣�����ADCУ׼��Ȼ��������DMA����ʹ��ADC��DMA��
*/
//ADC+DMA+TIME��������DMA ģʽ����ʱ(ADC_CFGR1 �Ĵ����е�DMAEN =1), ÿ��ת������ʱ�������һ��
//DMA �����������������ADC_DR �Ĵ����е�ת�����ݴ��͵����ָ����Ŀ���ַ�С�
//T15��DMA1���жϲ���Ҫ�ų�����ADC���ж�Ҳ����Ҫ��debug������
void InitADC_DMA(void) {

	DMA_InitTypeDef DMA_InitStruct;
	//NVIC_InitTypeDef  		 NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);			//����DMA1����ʱ�ӣ����ڶ�ȡADC1

	/*
	//DMA�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;       //ѡ��DMA1ͨ���ж�  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //�ж�ʹ��  
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;                //���ȼ���Ϊ0  
    NVIC_Init(&NVIC_InitStructure);  
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);		//ʹ��DMA�жϣ����濴���Ƿ���Ҫ�����˸о�����Ҫ
    */

	//DMA��ʼ��
	//������SYSCFG_CFGR1��ADC_DMA_RMPλ��Ϊ0��ADC�ź�Channel1����һ�𣬶�1ʱ��Channel2����һ��(ADCҪôChannel1Ҫô2)
	//�����������û���ù���resetֵΪ0�����Բ��ù�
	DMA_DeInit(DMA1_Channel1);					//ѡ��Ƶ��
	DMA_StructInit(&DMA_InitStruct);                            		//��ʼ��DMA�ṹ��  
	DMA_InitStruct.DMA_PeripheralBaseAddr = (UINT32)(&(ADC1->DR)); 		//���������ַ
	DMA_InitStruct.DMA_MemoryBaseAddr = (UINT32)(&g_u16ADCValFilter[0]);	//�����ڴ�ӳ���ַ
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;						//���ݴ��䷽��0�����������1���Ӵ洢����
	DMA_InitStruct.DMA_BufferSize = ADC_NUM;						//���������DMA���������С����
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//�����ַ���䣬�����̫�����ĸ������ַ
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;				//�ڴ��ַ����
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//������ִ���16λ
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//�ڴ���ִ���16λ
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;								//ѭ��ģʽ
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;							//�����ȼ�
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;									//���ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStruct);
	DMA_Cmd(DMA1_Channel1, ENABLE);	
}


void InitADC_GPIO(void) {

	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);	//����GPIOA������ʱ��
	
	//PA4_TTC_EV��PA5_VDC
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5; 	//ADC_Channel_5��PA5
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);					// PA5����ʱ������������

	//PB0_TTC_MOS1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0; 				//ADC_Channel_5��PA5
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStruct);					// PA5����ʱ������������
}

void InitADC_TIMER(void) {

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//NVIC_InitTypeDef  		 NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15,ENABLE); 		//ʱ��15ʹ��

	/*
	NVIC_InitStructure.NVIC_IRQChannel = TIM15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);					//ʹ��TIM15�ж� 
	*/

	//��ʱ��TIM15��ʼ��������ADC 10ms����
	TIM_TimeBaseStructure.TIM_Period = 1000; //1000*10 = 10ms
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/100000; //10us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM15,TIM_TRGOSource_Update);	//�ص㣬ѡ��TRGO����ԴΪ��ʱ������ʱ��
	TIM_Cmd(TIM15, ENABLE);  //ʹ��TIMx
}


//Ӧ���ǣ�Time����ADC������ADC������ɴ���DMA����
void InitADC_ADC1(void) {

	ADC_InitTypeDef ADC_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);		//����ADC1����ʱ��

	//ADC��ʼ��
	ADC_DeInit(ADC1);												//ADC�ָ�Ĭ������
	ADC_StructInit(&ADC_InitStruct);								//��ʼ��ADC�ṹ��
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b; 			//����ADC1������ģʽ�·ֱ���Ϊ12bits
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;	 //��ֹ����ģʽ�����������Ļ���������DMA������DMA����������жϣ�ֹͣADC���
														 //��仰�����ٿ�������Ľ����ǲ���һ�κ������ת���ͻ�����ͣЪ������DMA����
														 //Ҳ����ת��һ�κ󣬽��Ž�����һ��ת��������������
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T15_TRGO;		//�ⲿ��������ΪTIM15
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;	//�����ش�����ʹ�ü�ʱ��
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; 			//ADC�����Ҷ���
	ADC_InitStruct.ADC_ScanDirection = ADC_ScanDirection_Upward;	//��ͨ��AD����ʹ�ã�����ɨ��0-18ͨ��  
	ADC_Init(ADC1, &ADC_InitStruct); 
	ADC_ChannelConfig(ADC1, ADC_Channel_4|ADC_Channel_5|ADC_Channel_8, 
									ADC_SampleTime_55_5Cycles);	//ADC��ת��ʱ��=����ʱ��+12.5��ADCʱ������(�ź���ת��ʱ��)
																		//�����ܹ�55.5+12.5 = 68��ADC���ڡ�ʱ��Ϊ 68/8 us
	ADC_OverrunModeCmd(ADC1, ENABLE);                   //ʹ�����ݸ���ģʽ
	ADC_GetCalibrationFactor(ADC1); 					//ADCУ׼
	ADC_Cmd(ADC1, ENABLE);								//ADCʹ��
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)); 	//�ȴ�ADEN(ADC1->CR)ʹ�ܡ�ADC_Cmd(ADC1, ENABLE)��

    ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular); 	//ADC����DMAѭ��ģʽ	
	ADC_DMACmd(ADC1, ENABLE);								//ADC��DMAʹ��
	ADC_StartOfConversion(ADC1);							//ADCת����ʼ
}


void ADC_Current_Smooth(void) {

}


//69.3����3.3 = 21
void ADC_Vbus(void) {

}


void ADC_TTC(void) {
	INT32 t_i32temp = 0;	
	
    //-------------Environment�¶�(+40)-------------
	t_i32temp = (INT32)g_u16ADCValFilter[ADC_TEMP_EV1];		// ��ȡADֵ
    t_i32temp = GetEndValue(iSheldTemp_10K, (UINT16)LENGTH_TBLTEMP_PORT_10K, (UINT16)t_i32temp);
    g_u32ADCValFilter2[ADC_TEMP_EV1] = (((t_i32temp << 10) - g_u32ADCValFilter2[ADC_TEMP_EV1]) >> 3) + g_u32ADCValFilter2[ADC_TEMP_EV1];
	g_i32ADCResult[ADC_TEMP_EV1] = (UINT16)((g_u32ADCValFilter2[ADC_TEMP_EV1] + 512)>>10);

	    //-------------Environment�¶�(+40)-------------
	t_i32temp = (INT32)g_u16ADCValFilter[ADC_TEMP_EV2];		// ��ȡADֵ
    t_i32temp = GetEndValue(iSheldTemp_10K, (UINT16)LENGTH_TBLTEMP_PORT_10K, (UINT16)t_i32temp);
    g_u32ADCValFilter2[ADC_TEMP_EV2] = (((t_i32temp << 10) - g_u32ADCValFilter2[ADC_TEMP_EV2]) >> 3) + g_u32ADCValFilter2[ADC_TEMP_EV2];
	g_i32ADCResult[ADC_TEMP_EV2] = (UINT16)((g_u32ADCValFilter2[ADC_TEMP_EV2] + 512)>>10);
	
	//-------------MOS1�¶�(+40)-------------
	t_i32temp = (INT32)g_u16ADCValFilter[ADC_TEMP_MOS1];		// ��ȡADֵ
    t_i32temp = GetEndValue(iSheldTemp_10K, (UINT16)LENGTH_TBLTEMP_PORT_10K, (UINT16)t_i32temp);
    g_u32ADCValFilter2[ADC_TEMP_MOS1] = (((t_i32temp << 10) - g_u32ADCValFilter2[ADC_TEMP_MOS1]) >> 3) + g_u32ADCValFilter2[ADC_TEMP_MOS1];
	g_i32ADCResult[ADC_TEMP_MOS1] = (UINT16)((g_u32ADCValFilter2[ADC_TEMP_MOS1] + 512)>>10);
	
}


void InitADC(void) {
	UINT8 i;
	InitADC_GPIO();
	InitADC_TIMER();
	InitADC_DMA();
	InitADC_ADC1();
	
	for(i = 0; i < ADC_NUM; i++) {
        g_u16ADCValFilter[i] = 0;
        g_i32ADCResult[i] = 0;
		g_u32ADCValFilter2[i] = 0;
    }
}


void App_AnlogCal(void) {
	if(STARTUP_CONT == System_FUNC_StartUp(SYSTEM_FUNC_STARTUP_ADC)) {
		//return;
	}

	if(0 == g_st_SysTimeFlag.bits.b1Sys1msFlag) {
		return;
	}

	//ADC_Vbus();
	ADC_TTC();
}
