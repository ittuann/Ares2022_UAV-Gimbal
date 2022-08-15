/*
 * messageQueue.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "messageQueue.h"
#include <stdbool.h>

QueueHandle_t messageQueue[QUEUE_NUM] = {NULL};	// ������Ϣ���о��
bool_t messageQueueCreateFlag = false;			// ��Ϣ���д�����ɱ�־λ

/**
  * @brief	��Ϣ���д���
  * @notice	������� MX_FREERTOS_Init ��
  */
void MessageQueueCreate(void)
{
	uint8_t i = 0;
	// �����Ϣ����
	for (i = 0; i < IMUANGLE; i ++ ) {
		messageQueue[i] =  xQueueCreate(1, 2 * sizeof(motor_measure_t *));	// ����FIFO�ĳ���Ϊ1 ָ��motor_measure_t�ṹ��ָ��Ķ���
	}
	// IMU��Ϣ����
	messageQueue[IMUANGLE] =  xQueueCreate(1, 3 * sizeof(float));
	messageQueue[IMUGYRO] =  xQueueCreate(1, 3 * sizeof(float));

	// У���Ƿ񴴽�ʧ��
	for (i = 0; i < QUEUE_NUM; i ++ ) {
		if (messageQueue[i] == NULL) {
			Error_Handler();
		}
	}

	messageQueueCreateFlag = true;
}

