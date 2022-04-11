/*
 * bsp_beep.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "bsp_beep.h"

extern TIM_HandleTypeDef htim4, htim5;

/**
  * @brief          ��ʼ��������
  */
void Beep_Init(void)
{
	HAL_TIM_Base_Start(&htim4);							    // ������ʱ��
	
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);				// ����PWMͨ��
	
	__HAL_TIM_PRESCALER(&htim4, 0);							// ���ö�ʱ����Ƶϵ��
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 0);
}

/**
  * @brief          ��ʼ��LED
  */
void LED_Init(void)
{
	HAL_TIM_Base_Start(&htim5);							    // ������ʱ��
	
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);				// ʹ��Ӧ��ʱ���Ķ�Ӧͨ����ʼPWM���
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
	
//	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, 65535);
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, 0);
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, 0);
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, 0);
}

/**
  * @brief          ��ʼ�����
  */
void Beep_Init_Success(void)
{
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 10000);
	HAL_Delay(50);
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 0);
	HAL_Delay(50);
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 10000);
	HAL_Delay(50);
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, 0);
}


/**
  * @brief          ��ʾRGB
  * @param[in]      aRGB:0xaaRRGGBB,'aa' ��͸����,'RR'�Ǻ�ɫ,'GG'����ɫ,'BB'����ɫ
  * notice			����ɫ������8λ16���Ʊ�ʾΪ0xFFFF0000
  */
void aRGB_led_show(uint32_t aRGB)
{
	uint8_t alpha = 0x00;
	uint16_t red = 0x00, green = 0x00, blue = 0x00;

    alpha = (aRGB & 0xFF000000) >> 24;
    red = ((aRGB & 0x00FF0000) >> 16) * alpha;
    green = ((aRGB & 0x0000FF00) >> 8) * alpha;
    blue = ((aRGB & 0x000000FF) >> 0) * alpha;

    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, blue);
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_2, green);
    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, red);
}
