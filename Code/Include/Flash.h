#ifndef FLASH_H
#define FLASH_H

//ϵͳ�洢����������ST����������ڲ�Ԥ����һ��BootLoader��Ҳ����ISP��������һ��ROM
//û��ʲô�е�������Ʒ��ֻ����C8��R8����Ϊ64K��	 ÿҳ1KB��ϵͳ�洢��3KB��
#define FLASH_ADDR_IAP_START 			0x08000000		//IAP=7K������ط�����һ�����⣬�޸ĺ����6K�����淴ӳ���������Zi-dataҲ��flash�Ķ���
#define FLASH_ADDR_APP_START 			0x08001C00		//APP=64-7-1-1=55K


// #define FLASH_ADDR_SOC_RTC_CNT          0x0800F000		//RTC����ͳ�ƣ������Ƿ����SOC

#define FLASH_ADDR_WAKE_TYPE            0x0800F400		//���ѷ�ʽ�ļ�¼
#define FLASH_ADDR_UPDATE_FLAG 			0x0800F800		//������־λ��1K
#define FLASH_ADDR_SLEEP_FLAG           0x0800FC00		//���߹ؼ�ָ�1K

#define FLASH_TO_IAP_VALUE				((UINT16)0x00AB)
#define FLASH_TO_APP_VALUE				((UINT16)0xFFFF)

#define FLASH_VALUE_WAKE_RTC            ((UINT16)0x1234)
#define FLASH_VALUE_WAKE_OTHER          ((UINT16)0xFFFF)

#define FLASH_NORMAL_SLEEP_VALUE    	((UINT16)0x1234)
#define FLASH_DEEP_SLEEP_VALUE    		((UINT16)0x1235)
#define FLASH_HICCUP_SLEEP_VALUE    	((UINT16)0x1236)
#define FLASH_SLEEP_RESET_VALUE    		((UINT16)0xFFFF)


#define MCU_RESET()	NVIC_SystemReset()


FLASH_Status FlashWriteOneHalfWord(uint32_t StartAddr,uint16_t Buffer);
UINT16 FlashReadOneHalfWord(UINT32 faddr);
void App_FlashUpdateDet(void);
void Init_IAPAPP(void);

#endif	/* FLASH_H */

