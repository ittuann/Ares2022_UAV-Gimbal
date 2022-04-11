/*
 * remote_control.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "remote_control.h"

extern UART_HandleTypeDef	huart3;
extern DMA_HandleTypeDef	hdma_usart3_rx;

RC_ctrl_t rc_ctrl;							// ң�������ݽṹ��
static LpfRC1st_t RC_CH_FIR[4];				// ң�����˲��ṹ��

uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];	// ����ԭʼ����, Ϊ18���ֽ�, ����36���ֽڳ���, ��ֹDMA����Խ��

/**
  * @brief          ң������ʼ��
  */
void Remote_Control_Init(void)
{
    RC_init(sbus_rx_buf[0], sbus_rx_buf[1], SBUS_RX_BUF_NUM);
}

/**
  * @brief          ң����D-BUSЭ�����
  * @notion			���ջ�ÿ��14msͨ��DBUS����һ֡18�ֽ�����
  * @notion			ң�������������λ 1��2��3�У������XYZ����ƶ��ٶ� �������ң���갴�� 0û����1����
  * @notion			ÿ�����̰�����Ӧһ��bit Bit0-W��Bit1-S��Bit2-A��Bit3-D��Bit4-Q��Bit5-E��Bit6-Shift��Bit7-Ctrl
  * @param[in]      sbus_buf: ԭ������ָ��
  * @param[out]     rc_ctrl: ң��������ָ
  * @retval         none
  */
void sbus_to_rc(volatile const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl)
{
    if (sbus_buf == 0 || rc_ctrl == 0) {
        return;
    }

    rc_ctrl->rc.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;        // ң����ͨ��0 ��ҡ�˺���
    rc_ctrl->rc.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff; // ң����ͨ��1 ��ҡ������
    rc_ctrl->rc.ch[2] = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) |
                         (sbus_buf[4] << 10)) &0x07ff;						// ң����ͨ��2 ��ҡ�˺���
    rc_ctrl->rc.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff; // ң����ͨ��3 ��ҡ������
    rc_ctrl->rc.s[0] = ((sbus_buf[5] >> 4) & 0x0003);                  		// ң��������� S1 ��࿪��λ
    rc_ctrl->rc.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2;					// ң��������� S2 �Ҳ࿪��λ
    rc_ctrl->mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);                    // �����X����ƶ��ٶ�
    rc_ctrl->mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);                    // �����Y����ƶ��ٶ�
    rc_ctrl->mouse.z = sbus_buf[10] | (sbus_buf[11] << 8);                  // �����Z����ƶ��ٶ�
    rc_ctrl->mouse.press_l = sbus_buf[12];                                  // �������Ƿ���
    rc_ctrl->mouse.press_r = sbus_buf[13];                                  // ����Ҽ��Ƿ���
    rc_ctrl->key.v = sbus_buf[14] | (sbus_buf[15] << 8);                    // ���̰���
    rc_ctrl->rc.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);                 // NULL�����ֶ�

    rc_ctrl->rc.ch[0] -= RC_CH_VALUE_OFFSET;								// ��ȥ��ֵ��Ϊ[-660, 660]
    rc_ctrl->rc.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[4] -= RC_CH_VALUE_OFFSET;
}

/**
  * @brief          ң�������������ƣ���Ϊң�����Ĳ����ϻ�������λ��ʱ��һ��Ϊ0
  */
#define rc_deadband_limit(input, output, dealine)        	\
{                                                    		\
        if ((input) > (dealine) || (input) < -(dealine)) {	\
            (output) = (input);                          	\
        } else {											\
            (output) = 0;                                	\
        }                                                	\
    }
	
/**
  * @brief          ����ң�����ֽ����
  */
void RC_Gimbal_Diagram(void)
{
	// ����ң����ֵ������������
	rc_deadband_limit(rc_ctrl.rc.ch[YAW_CHANNEL], RC_CH_FIR[YAW_CHANNEL].OriginData, GIMBAL_RC_DEADLINE);
    rc_deadband_limit(rc_ctrl.rc.ch[PITCH_CHANNEL], RC_CH_FIR[PITCH_CHANNEL].OriginData, GIMBAL_RC_DEADLINE);

    // ����ת��
    RC_CH_FIR[YAW_CHANNEL].OriginData = RC_CH_FIR[YAW_CHANNEL].OriginData * RC_Yaw_Sen;
    RC_CH_FIR[PITCH_CHANNEL].OriginData = RC_CH_FIR[PITCH_CHANNEL].OriginData * RC_Pitch_Sen;
	
	// һ�׵�ͨ�˲�����б����Ϊ�����ٶ�����
	LowPassFilterRC1st(&RC_CH_FIR[YAW_CHANNEL], 0.800f, RC_CH_FIR[YAW_CHANNEL].OriginData);
	LowPassFilterRC1st(&RC_CH_FIR[PITCH_CHANNEL], 0.800f, RC_CH_FIR[PITCH_CHANNEL].OriginData);
	
	// ����Ҫ�����Ӽ��� ����ֹ�ۼ����
	if (fabsf(RC_CH_FIR[YAW_CHANNEL].FilterData) < fabsf(GIMBAL_RC_DEADLINE * RC_Yaw_Sen)) {
		RC_CH_FIR[YAW_CHANNEL].FilterData = 0.0f;
	}
	if (fabsf(RC_CH_FIR[PITCH_CHANNEL].FilterData) < fabsf(GIMBAL_RC_DEADLINE * RC_Pitch_Sen)) {
		RC_CH_FIR[PITCH_CHANNEL].FilterData = 0.0f;
	}
	
	// ���
	PID_Mortor_Angle[AnglePID_GimbalPitch].EX_Val += RC_CH_FIR[PITCH_CHANNEL].FilterData;
//	PID_Mortor_Angle[AnglePID_GimbalYaw].EX_Val = RC_CH_FIR[YAW_CHANNEL].FilterData + PID_Mortor_Angle[AnglePID_GimbalYaw].Now_Val;
	PID_Mortor_Angle[AnglePID_GimbalYaw].EX_Val += RC_CH_FIR[YAW_CHANNEL].FilterData;
	PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].EX_Val += RC_CH_FIR[PITCH_CHANNEL].FilterData;

	// �޷�
	LIMIT(PID_Mortor_Angle[AnglePID_GimbalPitch].EX_Val, GimbalMachine_Pitch.Min_Angle, GimbalMachine_Pitch.Max_Angle);
	LIMIT(PID_Mortor_Angle[AnglePID_GimbalYaw].EX_Val, GimbalMachine_Yaw.Min_Angle, GimbalMachine_Yaw.Max_Angle);
	LIMIT(PID_Mortor_Angle[AnglePID_GimbalPitch_Relative].EX_Val, GimbalMachine_Pitch.Min_Angle, GimbalMachine_Pitch.Max_Angle);
}

/**
  * @brief          �����ٶȷֽ����
  */
void RC_Chassis_Diagram(void)
{
	float vx_set = 0.000f, vy_set = 0.000f, wz_set = 0.000f;

	// ����ң����ֵ������������
	rc_deadband_limit(rc_ctrl.rc.ch[CHASSIS_X_CHANNEL], vx_set, CHASSIS_RC_DEADLINE);
    rc_deadband_limit(rc_ctrl.rc.ch[CHASSIS_Y_CHANNEL], vy_set, CHASSIS_RC_DEADLINE);

    // ����ת��
	vx_set = vx_set *  CHASSIS_VX_RC_SEN;
	vy_set = vy_set * -CHASSIS_VY_RC_SEN;

	// һ�׵�ͨ�˲�����б����Ϊ�����ٶ�����
	vx_set = LowPassFilterRC1st(&RC_CH_FIR[CHASSIS_X_CHANNEL], 0.800f, vx_set);
	vy_set = LowPassFilterRC1st(&RC_CH_FIR[CHASSIS_Y_CHANNEL], 0.800f, vy_set);

	// ����Ҫ�����Ӽ��� ����ֹ�ۼ����
	if (fabsf(vx_set) < fabsf(CHASSIS_RC_DEADLINE *  CHASSIS_VX_RC_SEN)) {
		vx_set = 0;
	}
	if (fabsf(vy_set) < fabsf(CHASSIS_RC_DEADLINE * CHASSIS_VY_RC_SEN)) {
		vy_set = 0;
	}

	// ң����Z����һЩģʽ�¶Ե��̵�����
	if (chassis_behaviour_mode == CHASSIS_NO_FOLLOW_YAW || chassis_behaviour_mode == CHASSIS_OPEN) {
		// CHASSIS_NO_FOLLOW_YAW ģʽ
		rc_deadband_limit(rc_ctrl.rc.ch[CHASSIS_WZ_CHANNEL], wz_set, CHASSIS_RC_DEADLINE);
		wz_set = wz_set * -CHASSIS_WZ_RC_SEN;
		wz_set = LowPassFilterRC1st(&RC_CH_FIR[CHASSIS_WZ_CHANNEL], 0.800f, wz_set);
		if (fabsf(wz_set) < fabsf(CHASSIS_RC_DEADLINE * CHASSIS_WZ_RC_SEN)) {
			wz_set = 0;
		}
		// CHASSIS_OPEN ģʽ
		if (chassis_behaviour_mode == CHASSIS_OPEN) {
			vx_set = vx_set *  CHASSIS_OPEN_RC_SEN;
			vy_set = vy_set * -CHASSIS_OPEN_RC_SEN;
			wz_set = wz_set * -CHASSIS_OPEN_RC_SEN;
		}
	}

	// ����ĸ������˶��ֽ��������ٶ�
//	PID_Mortor_Speed[Chassis_M1].EX_Val = (vx_set + vy_set + wz_set); //ǰ����
//	PID_Mortor_Speed[Chassis_M2].EX_Val = (vx_set - vy_set - wz_set); //ǰ����
//	PID_Mortor_Speed[Chassis_M3].EX_Val = (vx_set + vy_set - wz_set); //������
//	PID_Mortor_Speed[Chassis_M4].EX_Val = (vx_set - vy_set + wz_set); //������
}

/**
  * @brief          ����ң����
  */
void RC_Restart(uint16_t dma_buf_num)
{
    __HAL_UART_DISABLE(&huart3);
    __HAL_DMA_DISABLE(&hdma_usart3_rx);

    hdma_usart3_rx.Instance->NDTR = dma_buf_num;

    __HAL_DMA_ENABLE(&hdma_usart3_rx);
    __HAL_UART_ENABLE(&huart3);
}

/**
  * @brief          �ر�ң����
  */
void RC_Unable(void)
{
    __HAL_UART_DISABLE(&huart3);
}
