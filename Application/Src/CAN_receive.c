/*
 * can_receive.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "can_receive.h"
#include <stdbool.h>

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

#if DEBUGMODE
	motor_measure_t motorData[MOTOR_NUM];	// ���е������
#endif


/**
  * @brief          �����������Э�����
  */
static void get_motor_measure(motor_measure_t *motor, uint8_t data[8])
{
	motor->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);
	motor->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);
	motor->torque_current = (uint16_t)((data)[4] << 8 | (data)[5]);
	motor->temperate = (data)[6];

	// ���λ�ù��㴦�� �������λ������ͻ������
//	if (motor->ecd - motor->ecd_last > 4096) {
//		motor->round--;
//	} else if (motor->ecd - motor->ecd_last < -4096) {
//		motor->round ++ ;
//	}
//	motor->position = motor->ecd + motor->round * 8192;
	// ������ٶȷ���ֵ���޷�������ת��Ϊ�з�������
//	if (motor->speed_rpm > 32768) {
//		motor->speed_rpm -= 65536;
//	}
}

/**
  * @brief			HAL��CAN FIFO0���������жϣ�Rx0���ص�����
  * @param			hcan : CAN���ָ��
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;	// �������������л�
	CAN_RxHeaderTypeDef RxHeader;							// CANͨ��Э��ͷ
	uint8_t rx_data[8] = {0};								// �ݴ�CAN��������
	motor_measure_t motorDataTmp;							// �������
	uint8_t i = 0;
	
	if (hcan == &hcan1)
	{
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rx_data) == HAL_OK)	// ����CAN�����Ϸ�����������
		{
			// ��Ӧ����������Ϸ��͵ķ����ı�ʶ���ͳ��������
			switch (RxHeader.StdId) {
				case CAN_2006_M_ID : i = MotorID_ShootM; break;
				case CAN_PITCH_MOTOR_ID : i = MotorID_GimbalPitch; break;
				case CAN_YAW_MOTOR_ID : i = MotorID_GimbalYaw; break;
				#if DEBUGMODE
					default : i = RxHeader.StdId - CAN_3508_M1_ID; break;
				#endif
			}

			// �����������Э�����
			get_motor_measure(&motorDataTmp, rx_data);
			#if DEBUGMODE
				get_motor_measure(&motorData[i], rx_data);
			#endif

			// ����Ϣ�������������
			if (messageQueueCreateFlag) {
				xQueueOverwriteFromISR(messageQueue[i], (void *)&motorDataTmp, &xHigherPriorityTaskWoken);
				portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			}
		}
	}
}

/**
  * @brief          HAL��CAN FIFO1���������жϣ�Rx1���ص�����
  */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
//	CAN_RxHeaderTypeDef RxHeader;
//	uint8_t i = 0;
//	uint8_t rx_data[8] = {0};
//
//	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader, rx_data) == HAL_OK)	//����CAN�����Ϸ�����������
//	{
//		i = RxHeader.StdId - CAN_3508_M1_ID;
//		get_motor_measure(&motorData[i], rx_data);
//	}
}

/**
  * @brief          ����C620������Ƶ���
  * @param[in]		motorID��Ӧ�ĵ�����Ƶ���, ��Χ [-16384, 16384]����Ӧ��������ת�ص�����Χ [-20A, 20A]
  * @param[in]      etcID: ���Ʊ��ı�ʶ��(���ID)Ϊ1-4����5-8
  */
void CAN_Cmd_C620(CAN_HandleTypeDef *hcan, int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4, bool_t etcID)
{
	CAN_TxHeaderTypeDef TxHeader;						// CANͨ��Э��ͷ
	uint8_t TxData[8] = {0};							// ���͵��ָ���
	uint32_t TxMailboxX = CAN_TX_MAILBOX0;				// CAN��������

	if (etcID == false) {
		TxHeader.StdId = CAN_3508_ALL_ID;				// ��׼��ʽ��ʶ��ID
	} else {
		TxHeader.StdId = CAN_3508_ETC_ID;
	}
	TxHeader.ExtId = 0;
	TxHeader.IDE = CAN_ID_STD;							// ��׼֡
	TxHeader.RTR = CAN_RTR_DATA;						// ����֡����Ϊ����֡
	TxHeader.DLC = 0x08;								// ���ݳ�����
	TxData[0] = (uint8_t)(motor1 >> 8);
	TxData[1] = (uint8_t)motor1;
	TxData[2] = (uint8_t)(motor2 >> 8);
	TxData[3] = (uint8_t)motor2;
	TxData[4] = (uint8_t)(motor3 >> 8);
	TxData[5] = (uint8_t)motor3;
	TxData[6] = (uint8_t)(motor4 >> 8);
	TxData[7] = (uint8_t)motor4;

	//�ҵ��յķ�������
	while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0);	// ��������������䶼�����˾͵ȴ�ֱ������ĳ���������
	if ((hcan->Instance->TSR & CAN_TSR_TME0) != RESET) {
		// ��鷢������0״̬ �������0���оͽ����������ݷ���FIFO0
		TxMailboxX = CAN_TX_MAILBOX0;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME1) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX1;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME2) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX2;
	}
	// ������ͨ��CAN���߷���
	#if DEBUGMODE
		if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX) != HAL_OK) {
			Error_Handler();							// ���CAN��Ϣ����ʧ���������ѭ��
		}
	#else
		HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX);
	#endif
}

/**
  * @brief          ����GM6020������Ƶ�ѹ
  * @param[in]      motorID��Ӧ��GM6020������Ƶ�ѹ����Χ [-30000, 30000]
  * @param[in]      etcID: ���Ʊ��ı�ʶ��Ϊ5-8����9-11
  */
void CAN_Cmd_GM6020(CAN_HandleTypeDef *hcan, int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4, bool_t etcID)
{
	CAN_TxHeaderTypeDef TxHeader;						// CANͨ��Э��ͷ
	uint8_t TxData[8] = {0};							// ���յ�����ݻ���
	uint32_t TxMailboxX = CAN_TX_MAILBOX0;				// CAN��������

	if (etcID == false) {
		TxHeader.StdId = CAN_6020_ALL_ID;				// ��׼��ʽ��ʶ��ID
	} else {
		TxHeader.StdId = CAN_6020_ETC_ID;				// ��׼��ʽ��ʶ��ID
	}
	TxHeader.ExtId = 0;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 0x08;
	TxData[0] = (uint8_t)(motor1 >> 8);
	TxData[1] = (uint8_t)motor1;
	TxData[2] = (uint8_t)(motor2 >> 8);
	TxData[3] = (uint8_t)motor2;
	TxData[4] = (uint8_t)(motor3 >> 8);
	TxData[5] = (uint8_t)motor3;
	TxData[6] = (uint8_t)(motor4 >> 8);
	TxData[7] = (uint8_t)motor4;

	//�ҵ��յķ������� �����ݷ��ͳ�ȥ
	while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0);	// ��������������䶼�����˾͵ȴ�ֱ������ĳ���������
	if ((hcan->Instance->TSR & CAN_TSR_TME0) != RESET) {
		// ��鷢������0״̬ �������0���оͽ����������ݷ���FIFO0
		TxMailboxX = CAN_TX_MAILBOX0;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME1) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX1;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME2) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX2;
	}
	// ������ͨ��CAN���߷���
	#if DEBUGMODE
		if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX) != HAL_OK) {
			Error_Handler();							// ���CAN��Ϣ����ʧ���������ѭ��
		}
	#else
		HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX);
	#endif
}

/**
  * @brief          ����GM3510������Ƶ�ѹ
  * @param[in]      motorID��Ӧ��GM6020������Ƶ�ѹ����Χ [-29000, 29000]
  * @param[in]      id: 1/2/3
  */
void CAN_Cmd_GM3510(CAN_HandleTypeDef *hcan, int16_t motor1, int16_t motor2, int16_t motor3)
{
	CAN_TxHeaderTypeDef TxHeader;						// CANͨ��Э��ͷ
	uint8_t TxData[8] = {0};							// ���յ�����ݻ���
	uint32_t TxMailboxX = CAN_TX_MAILBOX0;				// CAN��������

	TxHeader.StdId = 0x1FF;								// ��׼��ʽ��ʶ��ID
	TxHeader.ExtId = 0;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.DLC = 0x08;
	TxData[0] = (uint8_t)(motor1 >> 8);
	TxData[1] = (uint8_t)motor1;
	TxData[2] = (uint8_t)(motor2 >> 8);
	TxData[3] = (uint8_t)motor2;
	TxData[4] = (uint8_t)(motor3 >> 8);
	TxData[5] = (uint8_t)motor3;

	//�ҵ��յķ������� �����ݷ��ͳ�ȥ
	while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0);	// ��������������䶼�����˾͵ȴ�ֱ������ĳ���������
	if ((hcan->Instance->TSR & CAN_TSR_TME0) != RESET) {
		// ��鷢������0״̬ �������0����
		TxMailboxX = CAN_TX_MAILBOX0;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME1) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX1;
	} else if ((hcan->Instance->TSR & CAN_TSR_TME2) != RESET) {
		TxMailboxX = CAN_TX_MAILBOX2;
	}
	// ������ͨ��CAN���߷���
	#if DEBUGMODE
		if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX) != HAL_OK) {
			Error_Handler();							// ���CAN��Ϣ����ʧ���������ѭ��
		}
	#else
		HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, (uint32_t *)TxMailboxX);
	#endif
}

/**
  * @brief          ����IDΪ0x700��CAN��,��������3508��������������ID
  */
void CAN_cmd_chassis_reset_ID(void)
{
	CAN_TxHeaderTypeDef chassis_tx_header;				// CANͨ��Э��ͷ
	uint8_t	chassis_TxData[8] = {0};					// ���͵��ָ���
	chassis_tx_header.StdId = 0x700;
    chassis_tx_header.ExtId = 0;
    chassis_tx_header.IDE = CAN_ID_STD;
    chassis_tx_header.RTR = CAN_RTR_DATA;
    chassis_tx_header.DLC = 0x08;
    if (HAL_CAN_AddTxMessage(&CHASSIS_CAN, &chassis_tx_header, chassis_TxData, (uint32_t*)CAN_TX_MAILBOX0) != HAL_OK){
    	Error_Handler();								// ���CAN��Ϣ����ʧ���������ѭ��
    }
}

/**
  * @brief          ��ֹ�����ʼ����ת
  */
void Motor_Prevent_InitMadness(void)
{
	CAN_Cmd_C620(&CHASSIS_CAN, 0, 0, 0, 0, 0);
	HAL_Delay(10);
	CAN_Cmd_C620(&SHOOT_CAN, 0, 0, 0, 0, 1);
	HAL_Delay(10);
	CAN_Cmd_GM6020(&GIMBAL_CAN, 0, 0, 0, 0, 0);
	HAL_Delay(10);
	CAN_Cmd_GM6020(&GIMBAL_CAN, 0, 0, 0, 0, 1);
	HAL_Delay(10);
	CAN_Cmd_GM3510(&GIMBAL_CAN, 0, 0, 0);
}

