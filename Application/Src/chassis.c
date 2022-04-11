/*
 * chassis.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "chassis.h"

//static motor_measure_t MotorChassis[4];		// ���̵��������Ϣ����
chassis_behaviour_e chassis_behaviour_mode = CHASSIS_ZERO_FORCE, chassis_behaviour_last;	// ������Ϊģʽ
chassis_t chassis;							// ��������״̬

//#if DEBUGMODE
//	static Frequency_t Chassis_Freqency;
//	static float Chassis_Freq = 0.000f;		// ������������Ƶ��
//#endif


/**
  * @brief			ң�������ؿ��Ƶ���ģʽ
  */
void Chassis_Set_Mode(void)
{
	if (switch_is_up(rc_ctrl.rc.s[RC_SWLeft])) {
		chassis_behaviour_mode = CHASSIS_ZERO_FORCE;
	} else if (switch_is_mid(rc_ctrl.rc.s[RC_SWLeft])) {
		chassis_behaviour_mode = CHASSIS_NO_MOVE;
	} else if (switch_is_down(rc_ctrl.rc.s[RC_SWLeft])) {
		chassis_behaviour_mode = CHASSIS_NO_FOLLOW_YAW;
	} else {
		chassis_behaviour_mode = CHASSIS_ZERO_FORCE;
	}
}

/**
  * @brief			���̲������ݸ���
  */
void Chassis_Feedback_Update(void)
{
//	chassis.vx = (- MotorChassis[Chassis_M1].ecd + MotorChassis[Chassis_M2].ecd + MotorChassis[Chassis_M3].ecd - MotorChassis[Chassis_M4].ecd) * 0.25f * M3508_MOTOR_RPM_TO_VECTOR;
//	chassis.vy = (- MotorChassis[Chassis_M1].ecd - MotorChassis[Chassis_M2].ecd + MotorChassis[Chassis_M3].ecd + MotorChassis[Chassis_M4].ecd) * 0.25f * M3508_MOTOR_RPM_TO_VECTOR;
//	chassis.wz = (- MotorChassis[Chassis_M1].ecd - MotorChassis[Chassis_M2].ecd - MotorChassis[Chassis_M3].ecd - MotorChassis[Chassis_M4].ecd) * 0.25f * M3508_MOTOR_RPM_TO_VECTOR;
}

/**
  * @brief			���̿�������
  */
void ChassisTask(void const * argument)
{
//	portTickType xLastWakeTime;
//	const portTickType xFrequency = 2 / portTICK_RATE_MS;	// ��ʱ2ms
//
//	// �õ�ǰtickʱ���ʼ��pxPreviousWakeTime
//	xLastWakeTime = xTaskGetTickCount();
//
//	while(1)
//	{
//		// ���������ʱ
//		vTaskDelayUntil(&xLastWakeTime, xFrequency);
//
//		// ����Ϣ�����л�ȡ����
//		xQueueReceive(messageQueue[Chassis_M1], &MotorChassis[Chassis_M1], (1 / portTICK_RATE_MS));
//		xQueueReceive(messageQueue[Chassis_M2], &MotorChassis[Chassis_M2], (1 / portTICK_RATE_MS));
//		xQueueReceive(messageQueue[Chassis_M3], &MotorChassis[Chassis_M3], (1 / portTICK_RATE_MS));
//		xQueueReceive(messageQueue[Chassis_M4], &MotorChassis[Chassis_M4], (1 / portTICK_RATE_MS));
//
//		// ң�����������õ��̿���ģʽ
//		Chassis_Set_Mode();
//
//		// �л�ģʽʱ����ͱ���״̬
//		if (chassis_behaviour_mode != chassis_behaviour_last) {
//			PID_Clear(&PID_Mortor_Speed[SpeedPID_Chassis_M1]);
//			PID_Clear(&PID_Mortor_Speed[SpeedPID_Chassis_M2]);
//			PID_Clear(&PID_Mortor_Speed[SpeedPID_Chassis_M3]);
//			PID_Clear(&PID_Mortor_Speed[SpeedPID_Chassis_M4]);
//		}
//
//		// ���̲�ͬģʽ�µĿ���
//		if (chassis_behaviour_mode == CHASSIS_ZERO_FORCE) {
//			PID_Mortor_Speed[SpeedPID_Chassis_M1].Output = PID_Mortor_Speed[SpeedPID_Chassis_M2].Output
//			= PID_Mortor_Speed[SpeedPID_Chassis_M3].Output = PID_Mortor_Speed[SpeedPID_Chassis_M4].Output = 0;
//		} else if (chassis_behaviour_mode == CHASSIS_OPEN) {
//			RC_Chassis_Diagram();
//			PID_Mortor_Speed[SpeedPID_Chassis_M1].Output = PID_Mortor_Speed[SpeedPID_Chassis_M1].EX_Val;
//			PID_Mortor_Speed[SpeedPID_Chassis_M2].Output = PID_Mortor_Speed[SpeedPID_Chassis_M2].EX_Val;
//			PID_Mortor_Speed[SpeedPID_Chassis_M3].Output = PID_Mortor_Speed[SpeedPID_Chassis_M3].EX_Val;
//			PID_Mortor_Speed[SpeedPID_Chassis_M4].Output = PID_Mortor_Speed[SpeedPID_Chassis_M4].EX_Val;
//		} else if (chassis_behaviour_mode == CHASSIS_NO_FOLLOW_YAW || chassis_behaviour_mode == CHASSIS_NO_MOVE) {
//			if (chassis_behaviour_mode == CHASSIS_NO_FOLLOW_YAW) {
//				RC_Chassis_Diagram();
//			} else if (chassis_behaviour_mode == CHASSIS_NO_MOVE) {
//				PID_Mortor_Speed[SpeedPID_Chassis_M1].EX_Val = PID_Mortor_Speed[SpeedPID_Chassis_M2].EX_Val = PID_Mortor_Speed[SpeedPID_Chassis_M3].EX_Val = PID_Mortor_Speed[SpeedPID_Chassis_M4].EX_Val = 0;
//			}
//
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_Chassis_M1], MotorChassis[Chassis_M1].speed_rpm, PID_Mortor_Speed[Chassis_M1].EX_Val);
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_Chassis_M2], MotorChassis[Chassis_M2].speed_rpm, PID_Mortor_Speed[Chassis_M2].EX_Val);
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_Chassis_M3], MotorChassis[Chassis_M3].speed_rpm, PID_Mortor_Speed[Chassis_M3].EX_Val);
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_Chassis_M4], MotorChassis[Chassis_M4].speed_rpm, PID_Mortor_Speed[Chassis_M4].EX_Val);
//		}
//
//		// ���͵�����Ƶ���
//		CAN_Cmd_Chassis((int16_t)(PID_Mortor_Speed[SpeedPID_Chassis_M1].Output), (int16_t)(PID_Mortor_Speed[SpeedPID_Chassis_M2].Output), (int16_t)(PID_Mortor_Speed[SpeedPID_Chassis_M3].Output), (int16_t)(PID_Mortor_Speed[SpeedPID_Chassis_M4].Output));
//
//		// ���̲������ݸ���
//		Chassis_Feedback_Update();
//		// ��¼�ϴο���ģʽ
//		chassis_behaviour_last = chassis_behaviour_mode;
//
//		#if DEBUGMODE
//			Chassis_Freq = GetFrequency(&Chassis_Freqency);
//		#endif
//	}
}
