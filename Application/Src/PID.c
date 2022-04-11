/*
 * PID.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "PID.h"

PIDTypeDef_t PID_Mortor_Speed[SpeedPID_NUM], PID_Mortor_Angle[AnglePID_NUM];
PIDTypeDef_t PID_IMU_Temp;

/*********** PIDϵ�� ***********/
// ����ٶȻ�PIDϵ��
static float PID_Speed_Param[SpeedPID_NUM][5] =	{{0.65f	, 0.10f	, 0	, 10240	, 5120},
												{0.65f	, 0.10f	, 0	, 10240	, 5120},
												{0.65f	, 0.10f	, 0	, 10240	, 8000},
												{120.0f	, 0.015f, 0	, 4000	, 300},
												{0.65f	, 0.10f	, 0	, 10240	, 8000}};
// ���λ�û�PIDϵ��
static float PID_Angle_Param[AnglePID_NUM][5] =	{{5		, 0.10f	, 0		, 25	, 5},
												{1.2f	, 0		, 0.20f	, 20	, 5},
												{1000	, 2.10f	, 600	, 10240 , 8000}};
// �������¶�PIDϵ��
const float PID_IMUTemp_Param[5] =				{200, 10, 0, 1000, 500};

/**
  * @brief          PID�ṹ���ʼ��
  */
void PID_Init(void)
{
	uint8_t i = 0;
	
	// �ٶȻ�
	for (i = 0; i < SpeedPID_NUM; i++) {
		PID_Mortor_Speed[i].Kp = PID_Speed_Param[i][0];
		PID_Mortor_Speed[i].Ki = PID_Speed_Param[i][1];
		PID_Mortor_Speed[i].Kd = PID_Speed_Param[i][2];
		PID_Mortor_Speed[i].out_Max = PID_Speed_Param[i][3];
		PID_Mortor_Speed[i].i_Max = PID_Speed_Param[i][4];
		if (PID_Mortor_Speed[i].Ki != 0.0f) {
			PID_Mortor_Speed[i].sum_Max = 1.15f * PID_Mortor_Speed[i].i_Max / PID_Mortor_Speed[i].Ki;
		}
		PID_Clear(&PID_Mortor_Speed[i]);	// ��ֹ�����ʼ�����ת
	}
	
	// λ�û�
	for (i = 0; i < AnglePID_NUM; i++) {
		PID_Mortor_Angle[i].Kp = PID_Angle_Param[i][0];
		PID_Mortor_Angle[i].Ki = PID_Angle_Param[i][1];
		PID_Mortor_Angle[i].Kd = PID_Angle_Param[i][2];
		PID_Mortor_Angle[i].out_Max = PID_Angle_Param[i][3];
		PID_Mortor_Angle[i].i_Max = PID_Angle_Param[i][4];
		if (PID_Mortor_Angle[i].Ki != 0.0f) {
			PID_Mortor_Angle[i].sum_Max = 1.15f * PID_Mortor_Angle[i].i_Max / PID_Mortor_Angle[i].Ki;
		}
		PID_Clear(&PID_Mortor_Angle[i]);	// ��ֹ�����ʼ�����ת
	}
	
	// �������¶Ȼ�
	PID_IMU_Temp.Kp = PID_IMUTemp_Param[0];
	PID_IMU_Temp.Ki = PID_IMUTemp_Param[1];
	PID_IMU_Temp.Kd = PID_IMUTemp_Param[2];
	PID_IMU_Temp.out_Max = PID_IMUTemp_Param[3];
	PID_IMU_Temp.i_Max = PID_IMUTemp_Param[4];
	if (PID_IMU_Temp.Ki != 0.0f) {
		PID_IMU_Temp.sum_Max = 1.15f * PID_IMU_Temp.i_Max / PID_IMU_Temp.Ki;
	}
	PID_Clear(&PID_IMU_Temp);
}

/**
  * @brief          ԭʼλ��ʽ PID ����
  * @param[out]		pid : PID�ṹ����ָ��
  * @param[in]		ref : ��ǰֵ
  * @param[in]		set : ����ֵ
  */
void PID_Calc_Original(PIDTypeDef_t *pid, float ref, float set)
{
	// �����ϴ����
	pid->Err_Last = pid->Err_Now;
	// �������
	pid->Now_Val = ref;
	pid->EX_Val = set;
	pid->Err_Now = pid->EX_Val - pid->Now_Val;
	pid->Err_Sum += pid->Err_Now;
	// λ��ʽ PID ����
	pid->Output_p = pid->Kp * pid->Err_Now;
	pid->Output_i = pid->Ki * pid->Err_Sum;
	pid->Output_d = pid->Kd * (pid->Err_Now - pid->Err_Last);
//	// ����ʽ PID ����
//	pid->Output_p = pid->Kp * (pid->Err_Now - pid->Err_Last);
//	pid->Output_i = pid->Ki * pid->Err_Now;
//	pid->Output_d = pid->Kd * (pid->Err_Now - 2 * pid->Err_Last + pid->Err_LastLast);
	// �������޷�
	LIMIT(pid->Output_i, -pid->i_Max, pid->i_Max);
	LIMIT(pid->Err_Sum, -pid->sum_Max, pid->sum_Max);
	pid->Output = pid->Output_p + pid->Output_i + pid->Output_d;
//	pid->Output += pid->Output_p + pid->Output_i + pid->Output_d;
	// ����޷�
	LIMIT(pid->Output, -pid->out_Max, pid->out_Max);
}

/**
  * @brief          λ��ʽ PID ����
  * @param[out]		pid : PID�ṹ����ָ��
  * @param[in]		ref : ��ǰֵ
  * @param[in]		set : ����ֵ
  */
void PID_Calc(PIDTypeDef_t *pid, float ref, float set)
{
	// ���ƾ�ֵ
	pid->Err_LastLast = pid->Err_Last;
	pid->Err_Last = pid->Err_Now;
	pid->Last_Val = pid->Now_Val;
	pid->EX_Last = pid->EX_Val;
	pid->Output_Last = pid->Output;
	pid->Output_dd = pid->Output_d;
	
	pid->Now_Val = ref;
	// ���΢������
	pid->Now_Val = pid->Now_Val * 0.950f + pid->Last_Val * (1.000f - 0.950f);
	pid->EX_Val = set;
	pid->Err_Now = pid->EX_Val - pid->Now_Val;
	// ƫ��΢������
	pid->Err_Now = pid->Err_Now * 0.850f + pid->Err_Last * (1.000f - 0.850f);
	pid->Err_Sum += pid->Err_Now;

	// λ��ʽ PID ����
	pid->Output_p = pid->Kp * pid->Err_Now;
	pid->Output_i = pid->Ki * pid->Err_Sum;
	pid->Output_d = pid->Kd * (pid->Err_Now - pid->Err_Last);
	// ΢�����ȫ΢��
	pid->Output_d = pid->Output_d * 0.850f + pid->Output_dd * (1.000f - 0.850f);
	LIMIT(pid->Err_Sum, -pid->sum_Max, pid->sum_Max);
	LIMIT(pid->Output_i, -pid->i_Max, pid->i_Max);
	// λ��ʽ PID ����
	pid->Output = pid->Output_p + pid->Output_i + pid->Output_d;
	// �������ȫ΢��
	pid->Output = pid->Output * 0.850f + pid->Output_Last * (1.000f - 0.850f);
	LIMIT(pid->Output, -pid->out_Max, pid->out_Max);
}

/**
  * @brief          ����ʽ PID ����
  * @param[out]		pid : PID�ṹ����ָ��
  * @param[in]		ref : ��ǰֵ
  * @param[in]		set : ����ֵ
  */
void PID_Calc_Incremental(PIDTypeDef_t *pid, float ref, float set)
{
	// ���ƾ�ֵ
	pid->Err_LastLast = pid->Err_Last;
	pid->Err_Last = pid->Err_Now;
	pid->Last_Val = pid->Now_Val;
	pid->EX_Last = pid->EX_Val;
	pid->Output_Last = pid->Output;
	pid->Output_dd = pid->Output_d;

	pid->Now_Val = ref;
	// ���΢������
	pid->Now_Val = pid->Now_Val * 0.950f + pid->Last_Val * (1.000f - 0.950f);
	pid->EX_Val = set;
	pid->Err_Now = pid->EX_Val - pid->Now_Val;
	// ƫ��΢������
	pid->Err_Now = pid->Err_Now * 0.850f + pid->Err_Last * (1.000f - 0.850f);
	pid->Err_Sum += pid->Err_Now;

	// ����ʽ PID ����
	pid->Output_p = pid->Kp * (pid->Err_Now - pid->Err_Last);
	pid->Output_i = pid->Ki * pid->Err_Now;
	pid->Output_d = pid->Kd * (pid->Err_Now - 2 * pid->Err_Last + pid->Err_LastLast);
	// ΢�����ȫ΢��
	pid->Output_d = pid->Output_d * 0.850f + pid->Output_dd * (1.000f - 0.850f);
	LIMIT(pid->Err_Sum, -pid->sum_Max, pid->sum_Max);
	LIMIT(pid->Output_i, -pid->i_Max, pid->i_Max);
	// ����ʽ PID ����
	pid->Output += pid->Output_p + pid->Output_i + pid->Output_d;
	// �������ȫ΢��
	pid->Output = pid->Output * 0.850f + pid->Output_Last * (1.000f - 0.850f);
	LIMIT(pid->Output, -pid->out_Max, pid->out_Max);
}

/**
  * @brief          PID �����ֵ
  * @param[out]		pid : PID�ṹ����ָ��
  * @notice			�����л�ģʽʱ����򱣴�״̬
  */
void PID_Clear(PIDTypeDef_t *pid)
{
    if (pid == 0) {
        return;
    }
	
	pid->Last_Val = pid->Now_Val = 0;
	pid->EX_Last = pid->EX_Val = 0;
	
	pid->Err_Now = pid->Err_ABS = pid->Err_Last = pid->Err_LastLast = 0;
	pid->Err_Sum = 0;
	pid->Output_p = pid->Output_i = pid->Output_d = pid->Output_dd = 0;
	pid->Output = pid->Output_Last = 0;
}
