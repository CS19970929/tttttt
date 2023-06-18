#include "main.h"

__IO uint32_t Timeout = LONG_TIMEOUT;

RegisterGroup Registers_AFE1;
struct stBq769x0_Read g_stBq769x0_Read_AFE1;

unsigned char startData_[80]; // �ֲ���ȫ�ֱ���
unsigned char BufferCRCC[30];

const unsigned int OVPThreshold = 4500;
const unsigned int UVPThreshold = 2000;
const unsigned char SCDDelay = SCD_DELAY_400us;
// const unsigned char SCDThresh = SCD_THRESH_178mV_89mV;
const unsigned char SCDThresh = SCD_THRESH_200mV_100mV;

const unsigned char OVDelay = OV_DELAY_2s;
const unsigned char UVDelay = UV_DELAY_4s;
const UINT16 iSheldTemp_10K_AFE[LENGTH_TBLTEMP_AFE_10K] = {
    //AD		(Temp+40)*10		
    7954	,	100	,	//-30
    7769	,	150	,	//-25
    7551	,	200	,	//-20
    7298	,	250	,	//-15
    7009	,	300	,	//-10
    6685	,	350	,	//-5
    6336	,	400	,	//0
    5951	,	450	,	//5
    5555	,	500	,	//10
    5142	,	550	,	//15
    4728	,	600	,	//20
    4319	,	650	,	//25
    3922	,	700	,	//30
    3542    ,	750	,	//35
    3183    ,	800	,	//40
    2850    ,	850	,	//45
    2541    ,	900	,	//50
    2263 	,	950	,	//55
    2010 	,	1000,	//60
    1784 	,	1050,	//65
    1581 	,	1100,	//70
    1401 	,	1150,	//75
    1241 	,	1200,	//80
    1101 	,	1250,	//85
    977 	,	1300,	//90
    868 	,	1350,	//95
    773 	,	1400,	//100
    689 	,	1450,	//105
};


#ifdef I2C_SIMULATION
// ����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT(); // sda�����
	IIC_SDA = 1;
	IIC_SCL = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 0; // ǯסI2C���ߣ�׼�����ͻ��������
}

// ����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT(); // sda�����
	IIC_SCL = 0;
	IIC_SDA = 0; // STOP:when CLK is high DATA change form low to high
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SDA = 1; // ����I2C���߽����ź�
	__delay_us(DELAY_US_IIC_AFE1);
}

// �ȴ�Ӧ���źŵ���
// ����ֵ��1������Ӧ��ʧ��
//         0������Ӧ��ɹ�
UINT8 IIC_Wait_Ack(void)
{
	UINT8 ucErrTime = 0;
	SDA_IN(); // SDA����Ϊ����
	IIC_SDA = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	while (READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0; // ʱ�����0
	__delay_us(DELAY_US_IIC_AFE1);
	return 0;
}
// ����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 0;
}
// ������ACKӦ��
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 1;
	__delay_us(DELAY_US_IIC_AFE1);
	IIC_SCL = 0;
}
// IIC����һ���ֽ�
// ���شӻ�����Ӧ��
// 1����Ӧ��
// 0����Ӧ��
void IIC_Send_Byte(UINT8 txd)
{
	UINT8 t;
	SDA_OUT();
	IIC_SCL = 0; // ����ʱ�ӿ�ʼ���ݴ���
	for (t = 0; t < 8; t++)
	{
		// IIC_SDA=(txd&0x80)>>7;
		if ((txd & 0x80) >> 7)
			IIC_SDA = 1;
		else
			IIC_SDA = 0;
		txd <<= 1;
		__delay_us(DELAY_US_IIC_AFE1); // ��TEA5767��������ʱ���Ǳ����
		IIC_SCL = 1;
		__delay_us(DELAY_US_IIC_AFE1);
		IIC_SCL = 0;
		__delay_us(DELAY_US_IIC_AFE1);
	}
}

// ��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
UINT8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN(); // SDA����Ϊ����
	for (i = 0; i < 8; i++)
	{
		IIC_SCL = 0;
		__delay_us(DELAY_US_IIC_AFE1);
		IIC_SCL = 1;
		receive <<= 1;
		if (READ_SDA)
			receive++;
		__delay_us(DELAY_US_IIC_AFE1);
	}
	if (!ack)
		IIC_NAck(); // ����nACK
	else
		IIC_Ack(); // ����ACK
	return receive;
}

int I2CSendBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ByteCount, unsigned int *SentByte)
{
	unsigned int NumberOfBytesSent = 0;
	unsigned char *DataPointer;
	DataPointer = DataBuffer;

	IIC_Start(); // ǯס������

	IIC_Send_Byte(I2CSlaveAddress << 1); // ����AFE��ַ
	if (1 == IIC_Wait_Ack())
	{
		*SentByte = NumberOfBytesSent;
		// System_ERROR_UserCallback(AFE1_ERROR);		//���������������û��Ҫ����Ȼ�ܶ�ط���Ҫ���ú��鷳��
		return 1; // return 1�����յĵط����ñ��
	}

	for (NumberOfBytesSent = 0; NumberOfBytesSent < ByteCount; NumberOfBytesSent++)
	{
		IIC_Send_Byte(*DataPointer); // ���͵͵�ַ
		if (1 == IIC_Wait_Ack())
		{
			*SentByte = NumberOfBytesSent;
			// System_ERROR_UserCallback(AFE1_ERROR);
			return 1;
		}
		DataPointer++;
	}
	IIC_Stop(); // ����һ��ֹͣ����
	*SentByte = NumberOfBytesSent;
	__delay_ms(1);
	return 0;
}

int I2CReadBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ExpectedByteNumber, unsigned int *NumberOfReceivedBytes)
{
	unsigned char *DataPointer;
	unsigned int *NumberOfReceivedBytesPointer;
	unsigned int i;

	NumberOfReceivedBytesPointer = NumberOfReceivedBytes;
	*NumberOfReceivedBytesPointer = 0;
	DataPointer = DataBuffer;

	IIC_Start();

	IIC_Send_Byte((I2CSlaveAddress << 1) | I2C_RW_R); // ����д����
	if (1 == IIC_Wait_Ack())
	{
		// System_ERROR_UserCallback(AFE1_ERROR);
		return 1;
	}

	for (i = 0; i < ExpectedByteNumber; i++)
	{
		if ((i + 1) < ExpectedByteNumber)
		{
			*DataPointer = IIC_Read_Byte(1);
		}
		else
		{
			*DataPointer = IIC_Read_Byte(0);
		}
		DataPointer++;
	}

	IIC_Stop(); // ����һ��ֹͣ����
	return 0;
}
#endif

#ifdef I2C_SYSTEM
void Init_I2C(void)
{

	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	/* I2C configuration */

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE); // ����I2C2����ʱ��

	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable; // 0: ģ�������˲�������
	I2C_InitStruct.I2C_DigitalFilter = 0x00;				   // �����˲����ر�
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_Timing = 0x00901D2B; // ��仰�Ҳ����ȣ���һ��excel��������ģ�8MHz
	// I2C_InitStruct.I2C_Timing = 0x30E3363D;							//48MHz
	/* I2C Peripheral Enable */
	I2C_Cmd(I2C2, ENABLE);
	/* Apply I2C configuration after enabling it */
	I2C_Init(I2C2, &I2C_InitStruct);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/*
	GPIO_PinAFConfig(GPIOF , GPIO_Pin_6, GPIO_AF_1);		//GPIOF�޸��ù��ܣ�Ĭ��I2C
	GPIO_PinAFConfig(GPIOF ,GPIO_Pin_7, GPIO_AF_1);
	*/
}

uint32_t TIMEOUT_UserCallback(void)
{
	/* Block communication and all processes */

	System_ERROR_UserCallback(ERROR_AFE1);
	return 1;
}

/*
��ε���I2C�����������
A��IO�˿ڿ��������⣬��һ��ģ��I2C����SCL�˿ڻ��ˣ���������ߵ�ƽ
B��I2C_TransferHandlingû�˽������ʲô�ã�����������Բ���start�źţ���ͬʱ�����ַ(���ֲᣡ���������˺ܾã�Ҳû������ʾ�����۲죡)
C�������ַ�����Զ����ƶ��������Լ��ֶ����ƣ��������ű��ˣ�Ҫ��ʾ����ȥ�۲���������Ƿ����Լ���Ҫ�ģ�
D��I2C_AutoEnd_Mode��I2C_Generate_Start_Write��I2C_Generate_Start_Read�˽�ã�����д�������Ӧ�ֽ����Զ�����END,ACK�źš�reloadģʽ�Ͳ���֮���
E��ò�Ƶ�ַ����1λ�Ϳ����ˣ�����Ҫ�Ӷ�д����λ��I2C_Generate_Start_Write��I2C_Generate_Start_Read���Զ��ӣ����Ǿ����Լ�����Ҳ��Ӱ�졣
*/

int I2CSendBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ByteCount, unsigned int *SentByte)
{

	unsigned int NumberOfBytesSent = 0;
	unsigned char *DataPointer;
	DataPointer = DataBuffer;

	Timeout = LONG_TIMEOUT;
	Feed_IWatchDog;
	while (I2C_GetFlagStatus(I2C_AFE, I2C_FLAG_BUSY) != RESET)
	{ // �ȴ����߲�æ
		if ((Timeout--) == 0)
		{
			*SentByte = NumberOfBytesSent;
			return TIMEOUT_UserCallback();
		}
	}

	I2C_TransferHandling(I2C_AFE, (I2CSlaveAddress << 1) | I2C_RW_W, ByteCount, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

	Feed_IWatchDog;
	while (ByteCount--)
	{
		Timeout = LONG_TIMEOUT;
		while (I2C_GetFlagStatus(I2C_AFE, I2C_ISR_TXIS) == RESET)
		{ // �����ڷ����жϣ���
			if ((Timeout--) == 0)
			{ // �ȴ���д���ݽ�д����Ĵ���
				*SentByte = NumberOfBytesSent;
				I2C_GenerateSTOP(I2C_AFE, ENABLE);
				return TIMEOUT_UserCallback();
			}
		}
		I2C_SendData(I2C_AFE, *DataPointer);
		DataPointer++;
	}

	/* Wait until STOPF flag is set */
	Timeout = LONG_TIMEOUT;
	Feed_IWatchDog;
	while (I2C_GetFlagStatus(I2C_AFE, I2C_ISR_STOPF) == RESET)
	{
		if ((Timeout--) == 0)
		{
			*SentByte = NumberOfBytesSent;
			I2C_GenerateSTOP(I2C_AFE, ENABLE);
			return TIMEOUT_UserCallback();
		}
	}
	I2C_ClearFlag(I2C_AFE, I2C_ICR_STOPCF);

	*SentByte = NumberOfBytesSent;
	return 0;
}

int I2CReadBytes(unsigned char I2CSlaveAddress, unsigned char *DataBuffer, unsigned int ExpectedByteNumber, unsigned int *NumberOfReceivedBytes)
{
	// unsigned long int DelayCounter = 0;
	unsigned char *DataPointer;
	unsigned int *NumberOfReceivedBytesPointer;

	NumberOfReceivedBytesPointer = NumberOfReceivedBytes;
	*NumberOfReceivedBytesPointer = 0; // Ŀǰû�õ�
	DataPointer = DataBuffer;

	Timeout = LONG_TIMEOUT;
	Feed_IWatchDog;
	while (I2C_GetFlagStatus(I2C_AFE, I2C_FLAG_BUSY) != RESET)
	{ // �ȴ����߲�æ
		if ((Timeout--) == 0)
		{ // ��ΪPIC��������д�ٶ���д������stop��־��TC��־λȥ��
			return TIMEOUT_UserCallback();
		}
	}

	I2C_TransferHandling(I2C_AFE, (I2CSlaveAddress << 1) | I2C_RW_R, ExpectedByteNumber, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
	// I2C_TransferHandling(I2C2, (I2CSlaveAddress<<1), ExpectedByteNumber,  I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
	Feed_IWatchDog;
	while (ExpectedByteNumber)
	{
		Timeout = LONG_TIMEOUT;
		while (I2C_GetFlagStatus(I2C_AFE, I2C_ISR_RXNE) == RESET)
		{ // �յ��ǿգ����
			if ((Timeout--) == 0)
			{
				return TIMEOUT_UserCallback();
			}
		}
		*DataPointer = I2C_ReceiveData(I2C_AFE);
		DataPointer++;
		ExpectedByteNumber--;
	}

	Timeout = LONG_TIMEOUT;
	Feed_IWatchDog;
	while (I2C_GetFlagStatus(I2C_AFE, I2C_ISR_STOPF) == RESET)
	{
		if ((Timeout--) == 0)
		{
			return TIMEOUT_UserCallback();
		}
	}
	I2C_ClearFlag(I2C_AFE, I2C_ICR_STOPCF);

	return 0;
}
#endif

int I2CWriteBlockWithCRC(unsigned char I2CSlaveAddress, unsigned char StartAddress, unsigned char *Buffer, unsigned char Length)
{
	unsigned char *Pointer;
	int i;
	unsigned int SentByte = 0;
	int result;

	Pointer = BufferCRCC;
	*Pointer = I2CSlaveAddress << 1;
	Pointer++;
	*Pointer = StartAddress;
	Pointer++;
	*Pointer = *Buffer;
	Pointer++;
	*Pointer = CRC8(BufferCRCC, 3, CRC_KEY);

	for (i = 1; i < Length; i++)
	{
		Pointer++;
		Buffer++;
		*Pointer = *Buffer;
		*(Pointer + 1) = CRC8(Pointer, 1, CRC_KEY);
		Pointer++;
	}

	result = I2CSendBytes(I2CSlaveAddress, BufferCRCC + 1, 2 * Length + 1, &SentByte);

	// free(BufferCRC);
	BufferCRCC[0] = 255;

	return result;
}

int I2CReadBlockWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char *Buffer, unsigned char Length)
{
	unsigned char TargetRegister = Register;
	unsigned int SentByte = 0;
	// unsigned char *ReadData = NULL;		//ȥ��NULL���԰�
	unsigned char *ReadData;
	unsigned int ReadDataCount = 0;
	unsigned char CRCInput[2];

	unsigned char CRCC = 0;
	int ReadStatus = 0;
	int WriteStatus = 0;
	int i;

	ReadData = startData_;

	WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);

	ReadStatus = I2CReadBytes(I2CSlaveAddress, ReadData, 2 * Length, &ReadDataCount);

	if (ReadStatus != 0 || WriteStatus != 0)
	{
		// free(StartData);
		startData_[0] = 255;

		return 1;
	}

	CRCInput[0] = (I2CSlaveAddress << 1) + 1;
	CRCInput[1] = *ReadData;

	CRCC = CRC8(CRCInput, 2, CRC_KEY);

	ReadData++;
	if (CRCC != *ReadData)
	{
		// free(StartData);
		startData_[0] = 255;
		return 1;
	}
	else
		*Buffer = *(ReadData - 1);

	for (i = 1; i < Length; i++)
	{
		ReadData++;
		CRCC = CRC8(ReadData, 1, CRC_KEY);
		ReadData++;
		Buffer++;

		if (CRCC != *ReadData)
		{
			// free(StartData);
			startData_[0] = 255;

			return 1;
		}
		else
			*Buffer = *(ReadData - 1);
	}

	// free(StartData);
	startData_[0] = 255;

	return 0;
}

int I2CWriteRegisterByteWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char Data)
{
	unsigned char DataBuffer[4];
	unsigned int SentByte = 0;

	DataBuffer[0] = I2CSlaveAddress << 1;
	DataBuffer[1] = Register;
	DataBuffer[2] = Data;
	DataBuffer[3] = CRC8(DataBuffer, 3, CRC_KEY);

	return (I2CSendBytes(I2CSlaveAddress, DataBuffer + 1, 3, &SentByte));
}

int I2CReadRegisterByteWithCRC(unsigned char I2CSlaveAddress, unsigned char Register, unsigned char *Data)
{
	unsigned char TargetRegister = Register;
	unsigned int SentByte = 0;
	unsigned char ReadData[2];
	unsigned int ReadDataCount = 0;
	unsigned char CRCInput[2];
	unsigned char CRCC = 0;
	int ReadStatus = 0;
	int WriteStatus = 0;

	WriteStatus = I2CSendBytes(I2CSlaveAddress, &TargetRegister, 1, &SentByte);

	ReadStatus = I2CReadBytes(I2CSlaveAddress, ReadData, 2, &ReadDataCount);

	if (ReadStatus != 0 || WriteStatus != 0)
	{
		return 1;
	}

	CRCInput[0] = (I2CSlaveAddress << 1) + 1;
	CRCInput[1] = ReadData[0];

	CRCC = CRC8(CRCInput, 2, CRC_KEY);

	if (CRCC != ReadData[1])
		return 1;

	*Data = ReadData[0];
	return 0;
}

int GetADCGainOffset(unsigned char I2CSlaveAddress)
{
	int result;
	result = I2CReadRegisterByteWithCRC(I2CSlaveAddress, ADCGAIN1, &(Registers_AFE1.ADCGain1.ADCGain1Byte));
	result += I2CReadRegisterByteWithCRC(I2CSlaveAddress, ADCGAIN2, &(Registers_AFE1.ADCGain2.ADCGain2Byte));
	result += I2CReadRegisterByteWithCRC(I2CSlaveAddress, ADCOFFSET, &(Registers_AFE1.ADCOffset));
	return result;
}

int ConfigureBqMaximo(unsigned char I2CSlaveAddress)
{
	int result = 0;
	unsigned char bqMaximoProtectionConfig[8];

	// result = I2CWriteBlockWithOutCRC(I2CSlaveAddress, SYS_CTRL1, &(Registers.SysCtrl1.SysCtrl1Byte), 8);
	// result += I2CReadBlockWithOutCRC(I2CSlaveAddress, SYS_CTRL1, bqMaximoProtectionConfig, 8);

	result = I2CWriteBlockWithCRC(I2CSlaveAddress, SYS_CTRL1, &(Registers_AFE1.SysCtrl1.SysCtrl1Byte), 8);
	result += I2CReadBlockWithCRC(I2CSlaveAddress, SYS_CTRL1, bqMaximoProtectionConfig, 8);

#if 1
	if ((bqMaximoProtectionConfig[0] != Registers_AFE1.SysCtrl1.SysCtrl1Byte) || (bqMaximoProtectionConfig[1] != Registers_AFE1.SysCtrl2.SysCtrl2Byte) || (bqMaximoProtectionConfig[2] != Registers_AFE1.Protect1.Protect1Byte) || (bqMaximoProtectionConfig[3] != Registers_AFE1.Protect2.Protect2Byte) || (bqMaximoProtectionConfig[4] != Registers_AFE1.Protect3.Protect3Byte) || (bqMaximoProtectionConfig[5] != Registers_AFE1.OVTrip) || (bqMaximoProtectionConfig[6] != Registers_AFE1.UVTrip) || (bqMaximoProtectionConfig[7] != Registers_AFE1.CCCfg))
	{
		result = 1;
	}
#endif
	return result;
}

// ���ģ��ǰ�˱����������alert�źţ�ֻҪ����ģ��ǰ�˱����CHG��DSG���ſ��Ʋ�������κ�����
// ʹ�ã��Ͱ�����������1�źš�
int InitialisebqMaximo(unsigned char I2CSlaveAddress)
{
	int result = 0;

	// ���ߴ��磬RTC���Ѳ���Ҫ��������Ȼ��ع���
	// ����Ҫ�տ�����ʱ���ó���־λ֮��Ҫ�Ļ�������Ȼ����RTC�ı�־λ����û�������ߣ���ĵط��Ͷϵ磬���������;����������
	// ����ͳ������ˡ��ɼ�������ѹ����
	gu8_WakeUp_Type = FlashReadOneHalfWord(FLASH_ADDR_WAKE_TYPE);
	if (gu8_WakeUp_Type == FLASH_VALUE_WAKE_RTC)
	{
		FlashWriteOneHalfWord(FLASH_ADDR_WAKE_TYPE, FLASH_VALUE_WAKE_OTHER);
		// return result;
	}

	// MOS״̬���ܱ�
	I2CReadRegisterByteWithCRC(DEVICE_ADDR_AFE1, SYS_CTRL2, &(Registers_AFE1.SysCtrl2.SysCtrl2Byte));

	Registers_AFE1.SysCtrl1.SysCtrl1Byte = 0;
	Registers_AFE1.SysCtrl1.SysCtrl1Bit.ADC_EN = 1;
	Registers_AFE1.SysCtrl1.SysCtrl1Bit.TEMP_SEL = 1;

	// Registers_AFE1.SysCtrl2.SysCtrl2Byte = 0;
	// Registers_AFE1.SysCtrl2.SysCtrl2Bit.CHG_ON = 1;		//ǿ�ƴ�һ����
	// Registers_AFE1.SysCtrl2.SysCtrl2Bit.DSG_ON = 1;		//ǿ�ƴ�һ����
	Registers_AFE1.SysCtrl2.SysCtrl2Bit.CC_EN = 1; // ������ȡ����

	Registers_AFE1.CCCfg = 0x19;

	// Registers_AFE1.Protect1.Protect1Bit.RSNS = 1;
	// Registers_AFE1.Protect1.Protect1Bit.SCD_DELAY = SCDDelay;    //70us
	// Registers_AFE1.Protect1.Protect1Bit.SCD_THRESH = SCDThresh;  //40A

	// Registers_AFE1.Protect2.Protect2Bit.OCD_DELAY = OCDDelay;    //8ms
	// Registers_AFE1.Protect2.Protect2Bit.OCD_THRESH = OCDThresh;  //20A

	// InitShortCur();

	Registers_AFE1.Protect3.Protect3Bit.OV_DELAY = OVDelay; // 2s
	Registers_AFE1.Protect3.Protect3Bit.UV_DELAY = UVDelay; // 4s

	result = GetADCGainOffset(I2CSlaveAddress);

	g_stBq769x0_Read_AFE1.f32Gain = (365 + ((Registers_AFE1.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers_AFE1.ADCGain2.ADCGain2Byte & 0xE0) >> 5)) / 1000.0;
	g_stBq769x0_Read_AFE1.i16Gain = 365 + ((Registers_AFE1.ADCGain1.ADCGain1Byte & 0x0C) << 1) + ((Registers_AFE1.ADCGain2.ADCGain2Byte & 0xE0) >> 5);
	Registers_AFE1.OVTrip = (unsigned char)((((unsigned short)((OVPThreshold - Registers_AFE1.ADCOffset) / g_stBq769x0_Read_AFE1.f32Gain + 0.5) - OV_THRESH_BASE) >> 4) & 0xFF);
	Registers_AFE1.UVTrip = (unsigned char)((((unsigned short)((UVPThreshold - Registers_AFE1.ADCOffset) / g_stBq769x0_Read_AFE1.f32Gain + 0.5) - UV_THRESH_BASE) >> 4) & 0xFF);
	result = ConfigureBqMaximo(I2CSlaveAddress);

	return result;
}

UINT8 App_AFEshutdown(void)
{
	UINT8 data = 0;
	// I2CReadRegisterByteWithCRC(DEVICE_ADDR_AFE1,0x04,&data);
	// data &= 0xfc;
	I2CWriteRegisterByteWithCRC(DEVICE_ADDR_AFE1, 0x04, data);
	data |= 1;
	I2CWriteRegisterByteWithCRC(DEVICE_ADDR_AFE1, 0x04, data);
	data = 0;
	data |= 2;
	I2CWriteRegisterByteWithCRC(DEVICE_ADDR_AFE1, 0x04, data);
	return 0;
}

// ��ʼ��IIC
void InitAFE1_F6F7(void)
{
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	/* I2C configuration */

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE); // ����I2C2����ʱ��

	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable; // 0: ģ�������˲�������
	I2C_InitStruct.I2C_DigitalFilter = 0x00;				   // �����˲����ر�
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	// I2C_InitStruct.I2C_Timing = 0x00901D2B;						//��仰�Ҳ����ȣ���һ��excel��������ģ�8MHz
	I2C_InitStruct.I2C_Timing = 0x30E3363D; // 48MHz
	/* I2C Peripheral Enable */
	I2C_Cmd(I2C2, ENABLE);
	/* Apply I2C configuration after enabling it */
	I2C_Init(I2C2, &I2C_InitStruct);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	InitialisebqMaximo(DEVICE_ADDR_AFE1);
}

// ��ʼ��IIC
void InitAFE1(void)
{
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	// RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);		//I2C1��Ҫ���
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	/* I2C configuration */
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable; // 0: ģ�������˲�������
	I2C_InitStruct.I2C_DigitalFilter = 0x00;				   // �����˲����ر�
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_Timing = 0x00901D2B; // ��仰�Ҳ����ȣ���һ��excel��������ģ�8MHz
	// I2C_InitStruct.I2C_Timing = 0x30E3363D;							//48MHz

	/* I2C Peripheral Enable */
	I2C_Cmd(I2C2, ENABLE);
	/* Apply I2C configuration after enabling it */
	I2C_Init(I2C2, &I2C_InitStruct);

	/* Connect PXx to I2C_SCL*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1); // ���AFѡ����оƬ�ֲ��reg��
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1); // ֻ��PA��PB�Ż���

	InitialisebqMaximo(DEVICE_ADDR_AFE1);

	InitShortCur();
}

int UpdateVoltageFromBqMaximo(unsigned char I2CSlaveAddress)
{
	int Result = 0, i = 0;
	// unsigned char *pRawADCData = NULL;
	unsigned char *pRawADCData;
	unsigned int iTemp = 0;
	long lTemp1 = 0;
	INT32 iTemp2 = 0;

	Result = I2CReadBlockWithCRC(I2CSlaveAddress, VC1_HI_BYTE, &(Registers_AFE1.VCell1.VCell1Byte.VC1_HI), 40);

	pRawADCData = &Registers_AFE1.VCell1.VCell1Byte.VC1_HI;
	for (i = 0; i < 20; i++)
	{
		if (i < 15)
		{ // Vcell
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0x3fff;
			lTemp1 = ((long)iTemp * g_stBq769x0_Read_AFE1.i16Gain) / 1000;
			lTemp1 += (signed char)(Registers_AFE1.ADCOffset);
			UPDNLMT16(lTemp1, 65535, 0);
			g_stBq769x0_Read_AFE1.u16VCell[i] = (UINT16)lTemp1;
		}
		else if (i == 15)
		{ // Vbat
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0xffff;
			lTemp1 = ((long)iTemp * g_stBq769x0_Read_AFE1.i16Gain * 4) / 1000;
			lTemp1 += (signed char)(Registers_AFE1.ADCOffset * SeriesNum);
			UPDNLMT16(lTemp1, 100000, 0);
			g_stBq769x0_Read_AFE1.u32VBat = lTemp1;
		}
		else if (i < 19)
		{ // Temp
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0x3fff;
			iTemp = GetEndValue(iSheldTemp_10K_AFE, (UINT16)LENGTH_TBLTEMP_AFE_10K, (UINT16)iTemp);
			/*
			g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16] = (((((INT32)iTemp) << 10) - g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16]) >> 3)\
				+ (g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16]);
			g_stBq769x0_Read_AFE1.u16TempBat[i - 16] = (UINT16)((g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16] + 512) >> 10);
			*/
			g_stBq769x0_Read_AFE1.u16TempBat[i - 16] = (UINT16)iTemp;
		}
		else
		{ // CC
			iTemp2 = ((INT32)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0xffff;
			// g_stBq769x0_Read.i32Current = (INT16)(iTemp2 * 8440 /5000);
			g_stBq769x0_Read_AFE1.u16Current = (UINT16)(iTemp2);
		}
		pRawADCData += 2;
	}

	return Result;
}

// ��Ϊ0-3part�����10msʱ�����⣬��2part�ܽ��200msʱ�������ǲ��ܽ��10msʱ��
int UpdateVoltageFromBqMaximo_Partition(unsigned char I2CSlaveAddress, UINT8 Part)
{
	int Result = 0, i = 0;
	// unsigned char *pRawADCData = NULL;
	unsigned char *pRawADCData;
	unsigned int iTemp = 0;
	long lTemp1 = 0;
	INT32 iTemp2 = 0;

	Result = I2CReadBlockWithCRC(I2CSlaveAddress, VC1_HI_BYTE + Part * 10, &(Registers_AFE1.VCell1.VCell1Byte.VC1_HI) + Part * 10, 10);

	pRawADCData = &(Registers_AFE1.VCell1.VCell1Byte.VC1_HI) + Part * 10;
	for (i = Part * 5; i < Part * 5 + 5; i++)
	{
		if (i < 15)
		{ // Vcell
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0x3fff;
			lTemp1 = ((long)iTemp * g_stBq769x0_Read_AFE1.i16Gain) / 1000;
			lTemp1 += (signed char)(Registers_AFE1.ADCOffset);
			UPDNLMT16(lTemp1, 65535, 0);
			g_stBq769x0_Read_AFE1.u16VCell[i] = (UINT16)lTemp1;
		}
		else if (i == 15)
		{ // Vbat
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0xffff;
			lTemp1 = ((long)iTemp * g_stBq769x0_Read_AFE1.i16Gain * 4) / 1000;
			lTemp1 += (signed char)(Registers_AFE1.ADCOffset * SeriesNum);
			UPDNLMT16(lTemp1, 100000, 0);
			g_stBq769x0_Read_AFE1.u32VBat = lTemp1;
		}
		else if (i < 19)
		{ // Temp
			iTemp = ((unsigned int)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0x3fff;
			iTemp = GetEndValue(iSheldTemp_10K_AFE, (UINT16)LENGTH_TBLTEMP_AFE_10K, (UINT16)iTemp);
			/*
			g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16] = (((((INT32)iTemp) << 10) - g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16]) >> 3)\
				+ (g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16]);
			g_stBq769x0_Read_AFE1.u16TempBat[i - 16] = (UINT16)((g_stBq769x0_Read_AFE1.i32TempBatFilter[i - 16] + 512) >> 10);
			*/
			g_stBq769x0_Read_AFE1.u16TempBat[i - 16] = (UINT16)iTemp;
		}
		else
		{ // CC
			iTemp2 = ((INT32)(*pRawADCData << 8) + *(pRawADCData + 1)) & 0xffff;
			// g_stBq769x0_Read.i32Current = (INT16)(iTemp2 * 8440 /5000);
			g_stBq769x0_Read_AFE1.u16Current = (UINT16)(iTemp2);
		}
		pRawADCData += 2;
	}

	return Result;
}
