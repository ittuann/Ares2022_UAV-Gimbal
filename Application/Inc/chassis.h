/*
 * chassis.h
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#ifndef CHASSIS_H
#define CHASSIS_H

#include "main.h"
#include "remote_control.h"
#include "can_receive.h"
#include "PID.h"

#define CHASSIS_VX_RC_SEN 		2.5f			// ң�������Ըñ���ϵ��
#define CHASSIS_VY_RC_SEN		2.5f			// ң�������Ըñ���ϵ��
#define CHASSIS_WZ_RC_SEN		2.5f			// �� CHASSIS_NO_FOLLOW_YAW ģʽ�� ң����yawң�˳��Ըñ���ϵ��
#define CHASSIS_OPEN_RC_SEN		1				// �� CHASSIS_OPEN ģʽ�� ң�������Ըñ���ϵ��
#define CHASSIS_ANGLE_Z_RC_SEN 	0.000002f		// �� CHASSIS_INFANTRY_FOLLOW_GIMBAL_YAW ģʽ�£�ң������yawң�����ӵ�����Ƕȵı���

#define M3508_MOTOR_RPM_TO_VECTOR 0.000415809748903494517209f	//m3508 rpmת���ɵ����ٶȣ�m/s���ı���

typedef enum {
	CHASSIS_ZERO_FORCE  = 0,			//��������, ��û�ϵ�����
	CHASSIS_NO_MOVE,					//���̱��ֲ���
	CHASSIS_OPEN,						//ң������ֵ���Ա����ɵ���ֵ ֱ�ӷ��͵�can������
	CHASSIS_NO_FOLLOW_YAW,				//���̲�������̨�Ƕ�
	CHASSIS_INFANTRY_FOLLOW_GIMBAL_YAW	//�����������̸�����̨
} chassis_behaviour_e;

typedef struct {
	int16_t vx;
	int16_t vy;
	int16_t wz;
} chassis_t;

extern chassis_behaviour_e chassis_behaviour_mode, chassis_behaviour_last;

extern	void Chassis_Set_Mode(void);
extern	void Chassis_Feedback_Update(void);

#endif
