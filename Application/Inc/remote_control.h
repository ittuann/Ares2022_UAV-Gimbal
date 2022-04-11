/*
 * remote_control.h
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

#include "main.h"
#include "user_lib.h"
#include "bsp_usart.h"
#include "chassis.h"
#include "gimbal.h"
#include "can_receive.h"

#define RC_FRAME_LENGTH 18u								// ң����ͨ��Э���ֽڳ���
#define SBUS_RX_BUF_NUM 36u

/* ----------------------- RC Channel Definition ---------------------------- */
#define RC_CH_VALUE_MIN         	((uint16_t)364)
#define RC_CH_VALUE_OFFSET      	((uint16_t)1024)
#define RC_CH_VALUE_MAX         	((uint16_t)1684)
#define CHASSIS_X_CHANNEL 			1					// ����ǰ���ң����ͨ������
#define CHASSIS_Y_CHANNEL 			0					// �������ҵ�ң����ͨ������
#define CHASSIS_WZ_CHANNEL			2					// ������ģʽ�£�����ͨ��ң����������ת
#define YAW_CHANNEL   				0					// yaw����ͨ��
#define PITCH_CHANNEL 				1					// pitch����ͨ��
#define CHASSIS_RC_DEADLINE			10					// ����ң������
#define GIMBAL_RC_DEADLINE			10					// ��̨ң������
/* ----------------------- RC Switch Definition ----------------------------- */
#define RC_SW_UP                	((uint16_t)1)
#define RC_SW_MID               	((uint16_t)3)
#define RC_SW_DOWN              	((uint16_t)2)
#define switch_is_up(s)         	(s == RC_SW_UP)
#define switch_is_mid(s)        	(s == RC_SW_MID)
#define switch_is_down(s)			(s == RC_SW_DOWN)
#define RC_SWLeft                	(1)
#define RC_SWRight                	(0)
/* ----------------------- PC Key Definition -------------------------------- */
#define KEY_PRESSED_OFFSET_W		((uint16_t)1 << 0)
#define KEY_PRESSED_OFFSET_S		((uint16_t)1 << 1)
#define KEY_PRESSED_OFFSET_A		((uint16_t)1 << 2)
#define KEY_PRESSED_OFFSET_D		((uint16_t)1 << 3)
#define KEY_PRESSED_OFFSET_SHIFT	((uint16_t)1 << 4)
#define KEY_PRESSED_OFFSET_CTRL		((uint16_t)1 << 5)
#define KEY_PRESSED_OFFSET_Q		((uint16_t)1 << 6)
#define KEY_PRESSED_OFFSET_E		((uint16_t)1 << 7)
#define KEY_PRESSED_OFFSET_R		((uint16_t)1 << 8)
#define KEY_PRESSED_OFFSET_F		((uint16_t)1 << 9)
#define KEY_PRESSED_OFFSET_G		((uint16_t)1 << 10)
#define KEY_PRESSED_OFFSET_Z		((uint16_t)1 << 11)
#define KEY_PRESSED_OFFSET_X		((uint16_t)1 << 12)
#define KEY_PRESSED_OFFSET_C		((uint16_t)1 << 13)
#define KEY_PRESSED_OFFSET_V		((uint16_t)1 << 14)
#define KEY_PRESSED_OFFSET_B		((uint16_t)1 << 15)

typedef struct __attribute__((packed))
{
        struct __attribute__((packed))
        {
                int16_t ch[5];		// ���ֵ1684 �м�ֵ1024 ��Сֵ 364
                char s[2];			// ���ֵ3 ��Сֵ1
        } rc;
        struct __attribute__((packed))
        {
                int16_t x;			// ���ֵ32767 ��Сֵ-32768 ��ֵֹ0
                int16_t y;			// ���ֵ32767 ��Сֵ-32768 ��ֵֹ0
                int16_t z;			// ���ֵ32767 ��Сֵ-32768 ��ֵֹ0
                uint8_t press_l;	// ���ֵ1 ��Сֵ0
                uint8_t press_r;	// ���ֵ1 ��Сֵ0
        } mouse;
        struct __attribute__((packed))
        {
                uint16_t v;			// λֵ��ʶ
        } key;
} RC_ctrl_t;

extern RC_ctrl_t rc_ctrl;

extern uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];

extern	void sbus_to_rc(volatile const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl);
extern	void Remote_Control_Init(void);
extern	void RC_Gimbal_Diagram(void);
extern	void RC_Chassis_Diagram(void);

extern	void RC_Restart(uint16_t dma_buf_num);
extern	void RC_Unable(void);

#endif
