#include "main.h"


UINT8 g_u8DataRxCnt;
UINT8 g_u8StopRxCnt;
UINT8 g_u8I2cTxD0Buff[I2C_BUFF_D0_SIZE];
UINT8 g_u8I2cTxD1Buff[I2C_BUFF_D1_SIZE];
UINT8 *g_pI2cTx;

UINT8 g_u8SR1 = 0;
UINT8 g_u8SR3 = 0;;

void I2cSlaver_DataInit(void);

void Init_I2CSlaver(void) {

}

void I2cSlaver_DataInit(void)
{
    UINT8 i = 0;
    
    g_u8DataRxCnt = 0;
    g_u8StopRxCnt = 0;
    for(i=0;i<I2C_BUFF_D0_SIZE;i++)
    {
        g_u8I2cTxD0Buff[i] = 0;
    }
    g_u8I2cTxD0Buff[0] = I2C_CMD_D0;

    for(i=0;i<I2C_BUFF_D1_SIZE;i++)
    {
        g_u8I2cTxD1Buff[i] = 0;
    }
    g_u8I2cTxD1Buff[0] = I2C_CMD_D1;

    g_pI2cTx = g_u8I2cTxD0Buff;
}


/******************************************************
*��������:CheckSumSum16
*��   ��:buf ҪУ�������
        len У�����ݵĳ�
*��   ��:У���
*��   ��:У���-16
*******************************************************/
UINT16 CheckSum16(UINT8 *buf,UINT8 len)
{
	UINT8    i=0;
	UINT16 Sum=0;

	for (i=0;i<len;i++)
	{
		Sum+=*buf++;
	}

	return Sum;
}


void I2cSlaver_DataPacketD0(void)
{
    UINT8 i = 0;
    UINT16 t_u16Temp = 0;

    i = 0;
    g_u8I2cTxD0Buff[i++] = I2C_CMD_D0;

    // ����ܵ�ѹ
    t_u16Temp = (UINT16)(g_stBq769x0_Read_AFE1.u32VBat / 10);
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ����
    t_u16Temp = g_stBq769x0_Read_AFE1.u16Current / 10;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ʣ������
    //t_u16Temp = SOC_cal_element.C*10/3600;
		t_u16Temp = 0;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // �������
    //t_u16Temp = (UINT16)((SOC_cal_element.C1 *10 /3600)&0xFFFF);
	t_u16Temp = 0;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ѭ������
    t_u16Temp = 0;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ��������
    t_u16Temp = 0x6345;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ����״̬
    t_u16Temp = 0x7335;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ����״̬-��
    t_u16Temp = 0x8365;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ����״̬
    t_u16Temp = 0x9345;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // ����汾
    t_u16Temp = 0xA31B;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // ʣ�������ٷֱ�
    t_u16Temp = 0xB31D;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // FET����״̬
    t_u16Temp = 0xC335;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // ��ش���
    t_u16Temp = 8;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // NTC����
    t_u16Temp = 3;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // NTC�¶�1
    t_u16Temp = 0xF3AC;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // NTC�¶�2
    t_u16Temp = 0x03EF;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // NTC�¶�3
    t_u16Temp = 0x13FD;
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);

    // У���
    t_u16Temp = CheckSum16(g_u8I2cTxD0Buff,i);
    g_u8I2cTxD0Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD0Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);
}


void I2cSlaver_DataPacketD1(void)
{
    UINT8 i = 0;
    UINT16 t_u16Temp = 0;

    i = 0;
    g_u8I2cTxD1Buff[i++] = I2C_CMD_D1;

    // Cell1��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[0];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell2��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[1];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell3��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[2];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell4��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[3];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);    

    // Cell5��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[4];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell6��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[5];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell7��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[6];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell8��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[7];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell9��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[8];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell10��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[9];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell11��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[10];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell12��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[11];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);    

    // Cell13��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[12];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell14��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[13];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell15��ѹ
    t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[14];
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // Cell16��ѹ
    //t_u16Temp = g_stBq769x0_Read_AFE1.u16VCell[15];//TODO 15
    t_u16Temp = 0;
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);

    // У���
    t_u16Temp = CheckSum16(g_u8I2cTxD1Buff,i);
    g_u8I2cTxD1Buff[i++] = (UINT8)(t_u16Temp & 0x00FF);
    g_u8I2cTxD1Buff[i++] = (UINT8)((t_u16Temp >> 8) & 0x00FF);
}



//�ӻ�û��ʱ����չ��NOSTRETCH��clock stretchingͨ����SCL����������ͣһ������.ֱ���ͷ�SCL��Ϊ�ߵ�ƽ,����ż�������
//��SR1�Ĵ����ó��Ľ��ۣ���CR1�Ĵ�����7λ
//�Ѿ�����������ˣ���Ĭ����slaveģʽ����ô�л��������������
//By default, it operates in slave mode. The interface automatically switches from slave to master, after it generates a START condition 
//and from master to slave, if an arbitration loss or a STOP generation occurs, allowing Multi-Master capability.


//�ؼ��Ĵ�����I2C1->SR3��Bit2 TRA: Transmitter/Receiver
//0: Data bytes received
//1: Data bytes transmitted
//This bit is set depending on R/W bit of address byte, at the end of total address phase.
//It is also cleared by hardware after detection of Stop condition (STOPF=1), repeated Start condition,
//loss of bus arbitration (ARLO=1), or when PE=0.
//�üĴ����Ŀ������£�
//In 7-bit addressing mode,
//�C To enter Transmitter mode, a master sends the slave address with LSB reset.
//�C To enter Receiver mode, a master sends the slave address with LSB set.
//It is also cleared by hardware after detection of Stop condition (STOPF=1), repeated Start condition,
//loss of bus arbitration (ARLO=1), or when PE=0.
//�ٴξ����������masterģʽ�µ��÷���������salverģʽ��######�Ƿ�������######�����ص㣬�����û˵��ʵ��ó��Ľ���
//����һ������IAR����������۲�Ĵ�����ֵ�Ǵ���ģ�Ҫ����ȷ�õ��Ĵ�����ֵ�����ø�ֵ��LiveWatch�۲�


//ACKΪSDA�͵�ƽ��NACKΪ�ߵ�ƽ
//���̣�
//A��if(I2C1->SR1&0x02)
//��λ���ȷ���Start�źţ�Ȼ�����һ��Adress(+Wr0)��TRA����Ϊ0(received mode)����ƥ�������if(I2C1->SR1&0x02)�����������ݺ�������һ��ACK

//B��else if((I2C1->SR1&0x40)&&(0 == (I2C1->SR3&0x04)))
//Ȼ�����CommandCode���������else if((I2C1->SR1&0x40)&&(0 == (I2C1->SR3&0x04)))����ȡ0xD0����0xD1�����ʱ�򣬰ѷ���ָ��ָ����Ӧ�����飬��һ��ACK

//C��if(I2C1->SR1&0x02)
//Ȼ����λ���ٷ�Sr����ʼ�źţ�TRAλ��Ӳ�������Ȼ��Adress(+Rr1)���ٴν���if(I2C1->SR1&0x02)�������ݺ�����TRA����λ1(Transmitter mode)����һ��ACK

//D��else if((I2C1->SR1&0x84)&&(I2C1->SR3&0x04))
//����else if((I2C1->SR1&0x84)&&(I2C1->SR3&0x04))����ʼ��ͣ�����ݣ���λ�����չ���Ӧ�������ֽں󣬷���NACK�����ٷ����ݡ��ٷ���һ��Stop�źţ�TRAλ��Ӳ�������������������ģʽ

//����:�����������Ŀǰ�ĸ��򣬹���TRA���λ��������ҪȨ�����ͣ����û�ã���Ŀǰ��ʵ����Ϊ�������ٴ�������д���롣
void I2C_Salve_Deal(void) {

}
