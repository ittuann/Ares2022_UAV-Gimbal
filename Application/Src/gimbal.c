/*
 * gimbal.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "gimbal.h"

#define NORMALIZE_ANGLE180(angle)	angle = ((angle) > 180) ? ((angle) - 360) : (((angle) < -180) ? (angle) + 360 : angle)

typedef enum {
	GIMBAL_PROTECT = 0,
	GIMBAL_RELATIVE_ANGLE,			// ��̨����ֵ����
	GIMBAL_ABSOLUTE_ANGLE,			// ��̨�����ǿ���
} gimbal_behaviour_e;
gimbal_behaviour_e GimbalBehaviour_Mode = GIMBAL_PROTECT, GimbalBehaviour_Last;	// ��̨��Ϊģʽ

static motor_measure_t MotorGimbal[2];							// ��̨�����Ϣ����
Gimbal_MachineTypeDef_t GimbalMachine_Pitch, GimbalMachine_Yaw;	// ��̨��е��λ����
static float Gimbal_Angle[3] = {0.0f};							// ��̨��̬��Ϣ���� roll������ pitch������ yaw�����/ƫ����
static float Gimbal_Gyro[3] = {0.0f};							// ��̨���ٶ���Ϣ����
float RC_Pitch_Sen	= -0.0000350f;								// ң����Pitch����ϵ��
float RC_Yaw_Sen	= -0.0000750f;								// ң����Yaw����ϵ��

#if DEBUGMODE
	static Frequency_t Gimbal_Freqency;
	static float Gimbal_Freq = 0.000f;							// ��̨��������Ƶ��
#endif


/**
  * @brief		��̨��е��ֵ����λ��ʼ��
  */
void GimbalMachineTypeDef_Init(void)
{
	GimbalMachine_Pitch.Max_Ecd = 4950;
	GimbalMachine_Pitch.Min_Ecd = 3850;
	GimbalMachine_Pitch.Middle_Ecd = 4300;
	GimbalMachine_Pitch.Min_Angle = (GimbalMachine_Pitch.Min_Ecd - GimbalMachine_Pitch.Middle_Ecd) * MOTOR_ECD_TO_RADPI;
	GimbalMachine_Pitch.Max_Angle = (GimbalMachine_Pitch.Max_Ecd - GimbalMachine_Pitch.Middle_Ecd) * MOTOR_ECD_TO_RADPI;

	GimbalMachine_Yaw.Max_Ecd = 5000;
	GimbalMachine_Yaw.Min_Ecd = 1200;
	GimbalMachine_Yaw.Middle_Ecd = 3100;
	GimbalMachine_Yaw.Min_Angle = (GimbalMachine_Yaw.Min_Ecd - GimbalMachine_Yaw.Middle_Ecd) * MOTOR_ECD_TO_RADPI;
	GimbalMachine_Yaw.Max_Angle = (GimbalMachine_Yaw.Max_Ecd - GimbalMachine_Yaw.Middle_Ecd) * MOTOR_ECD_TO_RADPI;
}

/**
  * @brief		ң�������ؿ�����̨ģʽ
  */
void RC_Gimbal_SetMode(void)
{
	if (switch_is_up(rc_ctrl.rc.s[RC_SWRight])) {
		GimbalBehaviour_Mode = GIMBAL_PROTECT;
	} else if (switch_is_mid(rc_ctrl.rc.s[RC_SWRight])) {
		GimbalBehaviour_Mode = GIMBAL_RELATIVE_ANGLE;
	} else if (switch_is_down(rc_ctrl.rc.s[RC_SWRight])) {
		GimbalBehaviour_Mode = GIMBAL_ABSOLUTE_ANGLE;
	} else {
		GimbalBehaviour_Mode = GIMBAL_PROTECT;
	}
}

/**
  * @brief		��̨��������
  */
void GimbalTask(void const * argument)
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = pdMS_TO_TICKS(1UL);	// ������ʱ1ms

	PID_Init();
	GimbalMachineTypeDef_Init();

	// �õ�ǰtickʱ���ʼ�� pxPreviousWakeTime
	xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		// ���������ʱ
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		// ����Ϣ�����л�ȡ����
		xQueueReceive(messageQueue[IMUANGLE], &Gimbal_Angle, (1UL / portTICK_RATE_MS));
		xQueueReceive(messageQueue[IMUGYRO], &Gimbal_Gyro, (1UL / portTICK_RATE_MS));
		xQueueReceive(messageQueue[MotorID_GimbalPitch], &MotorGimbal[Gimbal_Pitch], (1UL / portTICK_RATE_MS));
		xQueueReceive(messageQueue[MotorID_GimbalYaw], &MotorGimbal[Gimbal_Yaw], (1UL / portTICK_RATE_MS));

		// ң��������������̨����ģʽ
		RC_Gimbal_SetMode();

		// �л�ģʽʱ����ͱ���״̬
		if (GimbalBehaviour_Mode != GimbalBehaviour_Last) {
			PID_Clear(&PID_Mortor_Speed[SpeedPID_GimbalPitch]);
			PID_Clear(&PID_Mortor_Speed[SpeedPID_GimbalYaw]);
			PID_Clear(&PID_Mortor_Angle[AnglePID_GimbalPitch]);
			PID_Clear(&PID_Mortor_Angle[AnglePID_GimbalYaw]);
			PID_Clear(&PID_Mortor_Angle[AnglePID_GimbalPitch_Relative]);

			PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].Now_Val = GimbalMachine_Pitch.Min_Angle;
			PID_Mortor_Angle[AnglePID_GimbalPitch].Now_Val = GimbalMachine_Pitch.Min_Angle;
			PID_Mortor_Angle[AnglePID_GimbalYaw].Now_Val = GimbalMachine_Yaw.Min_Angle;
		}

		// ��̨��ͬģʽ�µĿ���
		if (GimbalBehaviour_Mode == GIMBAL_PROTECT) {
			PID_Mortor_Speed[SpeedPID_GimbalYaw].Output = PID_Mortor_Speed[SpeedPID_GimbalPitch].Output = 0;
			CAN_Cmd_GM3510(&GIMBAL_CAN, 0, 0, 0);
		} else if (GimbalBehaviour_Mode == GIMBAL_RELATIVE_ANGLE) {
			// ң�����ֽ�
			RC_Gimbal_Diagram();

			// �ǶȻ�
			PID_Calc(&PID_Mortor_Angle[AnglePID_GimbalPitch_Relative], ((MotorGimbal[Gimbal_Pitch].ecd - GimbalMachine_Pitch.Middle_Ecd) * MOTOR_ECD_TO_RADPI), PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].EX_Val);
			PID_Calc(&PID_Mortor_Angle[AnglePID_GimbalYaw], ((MotorGimbal[Gimbal_Yaw].ecd - GimbalMachine_Yaw.Middle_Ecd) * MOTOR_ECD_TO_RADPI), PID_Mortor_Angle[AnglePID_GimbalYaw].EX_Val);

			// �ٶȻ�
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_GimbalPitch], MotorGimbal[Gimbal_Pitch].speed_rpm, PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].Output);
			PID_Calc(&PID_Mortor_Speed[SpeedPID_GimbalYaw], MotorGimbal[Gimbal_Yaw].speed_rpm, PID_Mortor_Angle[AnglePID_GimbalYaw].Output);

			// ���͵�����Ƶ���
			CAN_Cmd_GM3510(&GIMBAL_CAN, (int16_t)PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].Output, 0, 0);
		} else if (GimbalBehaviour_Mode == GIMBAL_ABSOLUTE_ANGLE) {
			// ң�����ֽ�
			RC_Gimbal_Diagram();

			// ��̨�ǶȻ�
//			PID_Calc(&PID_Mortor_Angle[AnglePID_GimbalPitch], Gimbal_Angle[1], PID_Mortor_Angle[AnglePID_GimbalPitch].EX_Val);			// pitch	������ ��Y����ת
//			PID_Calc(&PID_Mortor_Angle[AnglePID_GimbalYaw], Gimbal_Angle[2], PID_Mortor_Angle[AnglePID_GimbalYaw].EX_Val);				// yaw		�����/ƫ���� ��Z����ת
			PID_Calc(&PID_Mortor_Angle[AnglePID_GimbalPitch_Relative], -Gimbal_Angle[1], PID_Mortor_Angle[AnglePID_GimbalPitch].EX_Val);

			// ��̨���ٶȻ�
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_GimbalPitch], Gimbal_Gyro[0], PID_Mortor_Angle[AnglePID_GimbalPitch].Output);
//			PID_Calc(&PID_Mortor_Speed[SpeedPID_GimbalYaw], Gimbal_Gyro[2], PID_Mortor_Angle[AnglePID_GimbalYaw].Output);

			// ���͵�����Ƶ���
//			CAN_Cmd_GM3510(&GIMBAL_CAN, (int16_t)PID_Mortor_Speed[SpeedPID_GimbalPitch].Output, 0, 0);
			CAN_Cmd_GM3510(&GIMBAL_CAN, (int16_t)PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].Output, 0, 0);

		}

		// ���͵�����Ƶ���
		CAN_Cmd_GM6020(&GIMBAL_CAN, (int16_t)(PID_Mortor_Speed[SpeedPID_GimbalYaw].Output), 0, 0, 0, 1);

		// ��¼�ϴο���ģʽ
		GimbalBehaviour_Last = GimbalBehaviour_Mode;

		#if DEBUGMODE
			Gimbal_Freq = GetFrequency(&Gimbal_Freqency);
		#endif
  }
}
