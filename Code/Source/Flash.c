#include "main.h"

#ifdef _IAP

// ����Ϊ48����.s�ļ����vector�����������û����������ѧϰһ����ôִ�е�
#if (defined(__CC_ARM))
__IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined(__ICCARM__))
#pragma location = 0x20000000
__no_init __IO uint32_t VectorTable[48];
#elif defined(__GNUC__)
__IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined(__TASKING__)
__IO uint32_t VectorTable[48] __at(0x20000000);
#endif

#endif

void Init_IAPAPP(void)
{
#ifdef _IAP
	UINT8 i;
	// ������仰�������޷�debug������reset�޷��մ��룬����ԭ��������룬����̳̣��������ô���
	// RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // �ж���������ӳ��ʹ��

	for (i = 0; i < 48; i++)
	{
		VectorTable[i] = *(__IO uint32_t *)(APPLICATION_ADDRESS + (i << 2));
	}
	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM); // Remap SRAM at 0x00000000
#endif
}

void App_FlashUpdateDet(void)
{
	if (1 == u8FlashUpdateFlag)
	{
		__delay_ms(10);
		u8FlashUpdateFlag = 0;
		MCU_RESET();
	}
}

UINT16 FlashReadOneHalfWord(UINT32 faddr)
{
	return *(vu16 *)faddr;
}

FLASH_Status FlashWriteOneHalfWord(uint32_t StartAddr, uint16_t Buffer)
{
	FLASH_Status result;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	while (FLASH_ErasePage(StartAddr) != FLASH_COMPLETE)
		;
	result = FLASH_ProgramHalfWord(StartAddr, Buffer);
	FLASH_Lock();
	return result;
}
