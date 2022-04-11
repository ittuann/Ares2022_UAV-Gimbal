/*
 *  ch100.h
 *
 *  Created on: 2021��11��5��
 *      Author: LBQ
 */

#include "ch100.h"

//extern UART_HandleTypeDef huart3;
//extern DMA_HandleTypeDef hdma_usart3_rx;

// Statement
#define CH100_DATA_PRE		(0x5A)
#define CH100_DATA_TYPE		(0xA5)
#define CH100_DATA_LABEL1	(0x91)
#define CH100_DATA_ID		1
#define CH100_DATA_HEADLEN	6
#define CH100_DATA_LEN		82

//static int16_t	CU2(uint8_t *p) {int16_t	u; memcpy(&u, p, 2); return u;}
//static int32_t	CU4(uint8_t *p) {int32_t	u; memcpy(&u, p, 4); return u;}
static float	CR4(uint8_t *p) {float		r; memcpy(&r, p, 4); return r;}

// Define
static uint8_t CH100_buf[CH100_DATA_LEN] = {0};	// ֡����
static float CH100_eul[3] = {0};				// Pitch Yaw Roll
static float CH100_acc[3] = {0};				// XYZ���ٶ� 1G
static float CH100_gyro[3] = {0};				// XYZ���ٶ� deg/s

#if DEBUGMODE
	static Frequency_t CH100_Frequency;
	static float CH100_Freq = 0.000f;			// ��Ч��������Ƶ��
#endif

/**
  * @brief			CH100��ʼ��
  */
void CH100_USART_Init(void)
{
	if (HAL_UART_Receive_DMA((UART_HandleTypeDef *)&huart3, (uint8_t *)CH100_buf, (uint16_t)CH100_DATA_LEN) != HAL_OK) {	// ����DMA�ж�
		Error_Handler();
	}
}

/**
  * @brief			CRCУ��
  */
static void crc16_update(uint16_t *currectCrc, const uint8_t *src, uint32_t lengthInBytes)
{
	uint32_t crc = *currectCrc;
	uint32_t j;
	for (j = 0; j < lengthInBytes; ++j) {
		uint32_t i;
		uint32_t byte = src[j];
		crc ^= byte << 8;
		for (i = 0; i < 8; ++i) {
			uint32_t temp = crc << 1;
			if (crc & 0x8000) {
				temp ^= 0x1021;
			}
			crc = temp;
		}
	}
	*currectCrc = crc;
}

/**
  * @brief			CH100����ͨѶЭ�����
  * @param[in]		data
  */
void CH100_Proc(uint8_t data)
{
	static uint16_t ch100_nbyte = 0;	// �ڼ�֡
    static uint16_t payload_len = 0;	// ����֡����
	static uint16_t ch100_crc0 = 0;		// ����֡��CRC
	uint16_t ch100_crc = 0;				// �յ����ݼ���CRC

    if (ch100_nbyte == 0 && data == CH100_DATA_PRE) {
    	// У�鲢��¼֡ͷ
    	CH100_buf[0] = CH100_DATA_PRE;
        ch100_nbyte = 1;
    } else if (ch100_nbyte == 1 && data == CH100_DATA_TYPE) {
    	// У�鲢��¼֡ͷ
    	CH100_buf[1] = CH100_DATA_TYPE;
        ch100_nbyte = 2;
    } else if (ch100_nbyte == 2) {
    	CH100_buf[2] = data;
        ch100_nbyte = 3;
    } else if (ch100_nbyte == 3) {
    	CH100_buf[3] = data;
    	payload_len = CH100_buf[2] | (CH100_buf[3] << 8);
    	ch100_nbyte = 4;
    	if (payload_len != (CH100_DATA_LEN - CH100_DATA_HEADLEN)) {
    		// У�����ݰ�����
            ch100_nbyte = 0;
            memset(&CH100_buf, 0, sizeof(CH100_buf));
    	}
    } else if (ch100_nbyte >= 4 && ch100_nbyte <= payload_len + 6) {
    	// �洢֡����
    	CH100_buf[ch100_nbyte] = data;
    	if (ch100_nbyte == 5) {
    		// ȡ��֡��Я��CRC
    		ch100_crc0 = CH100_buf[4] | (CH100_buf[5] << 8);
    	}
    	if (ch100_nbyte == 6 && CH100_buf[ch100_nbyte] != CH100_DATA_LABEL1) {
    		// У�����ݱ�ǩ
            ch100_nbyte = 0;
            memset(&CH100_buf, 0, sizeof(CH100_buf));
    	}
    	if (ch100_nbyte == 7 && CH100_buf[ch100_nbyte] != CH100_DATA_ID) {
    		// У��ģ��ID
            ch100_nbyte = 0;
            memset(&CH100_buf, 0, sizeof(CH100_buf));
    	}
    	if (ch100_nbyte != 0) {
    		ch100_nbyte++;
    	}
    } else if (ch100_nbyte == CH100_DATA_LEN + 1) {
    	// ����CRC
    	crc16_update(&ch100_crc, CH100_buf, 4);
		crc16_update(&ch100_crc, CH100_buf + 6, payload_len);
		// CRCУ��
		if (ch100_crc == ch100_crc0) {
			// ��������ͨѶЭ��
			CH100_eul[2] = CR4(CH100_buf + 54);		// Roll		�����
			CH100_eul[0] = CR4(CH100_buf + 58);		// Pitch	������
			CH100_eul[1] = CR4(CH100_buf + 62);		// Yaw		�����/ƫ����

			#if DEBUGMODE
				// ������Ч��������Ƶ��
				CH100_Freq = GetFrequency(&CH100_Frequency);
			#endif
		}
    	ch100_nbyte = 0;
    	memset(&CH100_buf, 0, sizeof(CH100_buf));
    } else {
        ch100_nbyte = 0;
        memset(&CH100_buf, 0, sizeof(CH100_buf));
    }
}

/**
  * @brief			CH100����ͨѶЭ�����
  * @param[in]		data
  */
uint8_t CH100_ProcAll(uint8_t *data)
{
	uint16_t ch100_crc0 = 0;		// ����֡��CRC
	uint16_t ch100_crc = 0;			// �յ����ݼ���CRC

	// ȡ��֡��Я��CRC
	ch100_crc0 =  CH100_buf[4] | (CH100_buf[5] << 8);
	// ����CRC
	crc16_update(&ch100_crc, CH100_buf, 4);
	crc16_update(&ch100_crc, CH100_buf + 6, CH100_DATA_LEN - CH100_DATA_HEADLEN);
	// CRCУ��
	if (ch100_crc == ch100_crc0) {
		// ��������ͨѶЭ��
		CH100_acc[0] =  CR4(CH100_buf + 18);	// ���ٶ�XYZ
		CH100_acc[1] =  CR4(CH100_buf + 22);
		CH100_acc[2] =  CR4(CH100_buf + 26);
		CH100_gyro[0] = CR4(CH100_buf + 30);	// ���ٶ�XYZ
		CH100_gyro[1] = CR4(CH100_buf + 34);
		CH100_gyro[2] = CR4(CH100_buf + 38);
		CH100_eul[2] = CR4(CH100_buf + 54);		// Roll		�����
		CH100_eul[0] = CR4(CH100_buf + 58);		// Pitch	������
		CH100_eul[1] = CR4(CH100_buf + 62);		// Yaw		�����/ƫ����

		#if DEBUGMODE
			// ������Ч��������Ƶ��
			CH100_Freq = GetFrequency(&CH100_Frequency);
		#endif

		return 1;
	} else {
		memset(&CH100_buf, 0, sizeof(CH100_buf));
		return 0;
	}
}

/**
  * @brief			�����жϷ���ص�����
  */
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;	// �������������л�
//
//	if (huart->Instance == USART3){
//		if (CH100_ProcAll(CH100_buf) == 1) {
//			// ����Ϣ�������������
//			xQueueOverwriteFromISR(messageQueue[ANGLE], (void *)&CH100_eul, &xHigherPriorityTaskWoken);
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//		} else {
//			// ���մ�λ����DMA
//			HAL_UART_DMAStop(&huart3);
//			HAL_Delay(3);
//			HAL_UART_Receive_DMA((UART_HandleTypeDef *)&huart3, (uint8_t *)CH100_buf, (uint16_t)CH100_DATA_LEN);
//		}
//	}
//}

