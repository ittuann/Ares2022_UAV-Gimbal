/*
 * messageQueue.h
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#ifndef MessageQueue_H
#define MessageQueue_H

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "assert.h"
#include "user_lib.h"
#include "can_receive.h"

enum MessageQueue_e {
	MotorID_ChassisM1 = 0,
	MotorID_ShootM,
	MotorID_GimbalPitch,
	MotorID_GimbalYaw,
	MOTOR_NUM,
    IMUANGLE = MOTOR_NUM,
	IMUGYRO,
    QUEUE_NUM
};

extern QueueHandle_t messageQueue[QUEUE_NUM];	// ������Ϣ���о��
extern bool_t messageQueueCreateFlag;			// ��Ϣ���д�����ɱ�־λ

extern void MessageQueueCreate(void);

#endif
