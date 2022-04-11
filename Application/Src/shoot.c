/*
 * shoot.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "shoot.h"

extern TIM_HandleTypeDef htim1, htim3;

#define SHOOT_L_TIM				htim1
#define SHOOT_R_TIM				htim1
#define SHOOT_LASER_TIM			htim3

#define SHOOT_L_TIM_CHANNEL		TIM_CHANNEL_2
#define SHOOT_R_TIM_CHANNEL		TIM_CHANNEL_3
#define SHOOT_LASER_TIM_CHANNEL	TIM_CHANNEL_3


typedef enum {
	DONOT_SHOOT = 0,
	SHOOT_NORMAL,		// �������
	SHOOT_SLOW,			// ���ٲ������
} shoot_behaviour_e;
shoot_behaviour_e ShootBehaviour_Mode = DONOT_SHOOT, ShootBehaviour_Last;	// ��̨��Ϊģʽ

static motor_measure_t MotorShoot[1];					// ������������Ϣ����
static Ramp_t Stir, SnailL, SnailR;						// �����������
static uint16_t SnailL_Set = 1000, SnailR_Set = 1000;	// ����

#if DEBUGMODE
	static Frequency_t Shoot_Freqency;
	static float Shoot_Freq = 0.000f;					// ǹ����������Ƶ��
#endif

/**
  * @brief		Snail���PWM��ʼ��
  */
void Snail_Init(void)
{
	HAL_TIM_Base_Start(&htim1);										// ������ʱ��
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);						// ʹ��Ӧ��ʱ���Ķ�Ӧͨ����ʼPWM���
	HAL_TIM_PWM_Start(&SHOOT_L_TIM, SHOOT_L_TIM_CHANNEL);
	HAL_TIM_PWM_Start(&SHOOT_R_TIM, SHOOT_R_TIM_CHANNEL);
	__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, 1000);
	__HAL_TIM_SetCompare(&SHOOT_L_TIM, SHOOT_L_TIM_CHANNEL, 1000);
	__HAL_TIM_SetCompare(&SHOOT_R_TIM, SHOOT_R_TIM_CHANNEL, 1000);
}

/**
  * @brief		����PWM��ʼ��
  */
void Laser_Init(void)
{
	HAL_TIM_Base_Start(&SHOOT_LASER_TIM);							// ������ʱ��
	HAL_TIM_PWM_Start(&SHOOT_LASER_TIM, SHOOT_LASER_TIM_CHANNEL);	// ʹ���ⶨʱ����Ӧͨ��PWM���
}

/**
  * @brief		��������Snail���PWM
  */
void Snail_Set(void)
{
	__HAL_TIM_SetCompare(&SHOOT_L_TIM, SHOOT_L_TIM_CHANNEL, (uint16_t)(RampCalc(&SnailL, SnailL_Set, 10)));
	__HAL_TIM_SetCompare(&SHOOT_R_TIM, SHOOT_R_TIM_CHANNEL, (uint16_t)(RampCalc(&SnailR, SnailR_Set, 10)));
}

void RC_Shoot_SetMode(void)
{
	if (switch_is_up(rc_ctrl.rc.s[RC_SWLeft])) {
		ShootBehaviour_Mode = DONOT_SHOOT;
	} else if (switch_is_mid(rc_ctrl.rc.s[RC_SWLeft])) {
		ShootBehaviour_Mode = DONOT_SHOOT;
	} else if (switch_is_down(rc_ctrl.rc.s[RC_SWLeft])) {
		ShootBehaviour_Mode = SHOOT_NORMAL;
	} else {
		ShootBehaviour_Mode = DONOT_SHOOT;
	}
}

/**
  * @brief		�����������
  */
void ShootTask(void const * argument)
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = pdMS_TO_TICKS(5UL);	// ������ʱ5ms

	// ���������
//	vTaskSuspendAll();

	SnailL.SetVal = SnailL.NowVal = SnailL_Set;
	SnailR.SetVal = SnailR.NowVal = SnailR_Set;

	// ���ѵ�����
//	xTaskResumeAll();

	// �õ�ǰtickʱ���ʼ�� pxPreviousWakeTime
	xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		// ���������ʱ
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		// ����Ϣ�����л�ȡ����
		xQueueReceive(messageQueue[MotorID_ShootM], &MotorShoot[0], (1 / portTICK_RATE_MS));

		// ң��������������̨����ģʽ
		RC_Shoot_SetMode();

		// �л�ģʽʱ����ͱ���״̬
		if (ShootBehaviour_Mode != ShootBehaviour_Last) {
			__HAL_TIM_SetCompare(&htim1, SHOOT_L_TIM_CHANNEL, 1000);
			__HAL_TIM_SetCompare(&htim1, SHOOT_R_TIM_CHANNEL, 1000);
			SnailL_Set = SnailR_Set = 1000;
			Stir.SetVal = Stir.NowVal = 0;
			SnailL.SetVal = SnailL.NowVal = SnailL_Set;
			SnailR.SetVal = SnailR.NowVal = SnailR_Set;
			PID_Clear(&PID_Mortor_Speed[SpeedPID_ShootM]);
		}

		if (ShootBehaviour_Mode == SHOOT_NORMAL) {
			// ����Ħ����
			SnailL_Set = SnailR_Set = 1200;
			Snail_Set();
			// ������PID����
			PID_Mortor_Speed[SpeedPID_ShootM].EX_Val = 1000;
			PID_Calc(&PID_Mortor_Speed[SpeedPID_ShootM], MotorShoot[0].speed_rpm, RampCalc(&Stir, PID_Mortor_Speed[SpeedPID_ShootM].EX_Val, 50));
		}

		// ���͵�����Ƶ��� ��Gimbal������

		// ��¼�ϴο���ģʽ
		ShootBehaviour_Last = ShootBehaviour_Mode;

//		CAN_Cmd_C620(&SHOOT_CAN, (int16_t)PID_Mortor_Speed[SpeedPID_ShootM].Output, 0, 0, 0, 1);

//		Wireless_Send();

		#if DEBUGMODE
			Shoot_Freq = GetFrequency(&Shoot_Freqency);
		#endif
	}
}

/**
  * @brief		���Կ�������
  */
void TestTask(void const * argument)
{
//	OLED_init();

	while(1)
	{
		osDelay(1);
	}
}
