/*
 * bsp_adc.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "bsp_adc.h"

volatile float ADCVrefintProportion = 8.0586080586080586080586080586081e-4f;

/**
  * @brief			��ȡADC����ֵ
  * @retval			none
  * @example		adcx_get_chx_value(&hadc1, ADC_CHANNEL_1);
  */
uint16_t adcx_get_chx_value(ADC_HandleTypeDef *ADCx, uint32_t ch)
{
	ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ch;							// ADCת��ͨ��
    sConfig.Rank = 1;								// ADC�������� ��ת��˳��
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;	// ADC����ʱ��

	HAL_ADC_ConfigChannel(ADCx, &sConfig);			// ����ADCͨ���ĸ�������ֵ

    HAL_ADC_Start(ADCx);							// ����ADC����

    HAL_ADC_PollForConversion(ADCx, 1);				// �ȴ�ADCת������

	if (HAL_IS_BIT_SET(HAL_ADC_GetState(ADCx), HAL_ADC_STATE_REG_EOC)) {	// �ж�ת����ɱ�־λ�Ƿ�����
		return (uint16_t)(HAL_ADC_GetValue(ADCx));	// ��ȡADCֵ
	} else {
		return 0;
	}
}

/**
  * @brief          ADC�ڲ��ο�У׼��ѹVrefint��ʼ��
  * @retval         none
  */
void ADC_Vrefint_Init(void)
{
    uint32_t total_vrefint = 0;
	
    for (uint8_t i = 0; i < 200; i ++ ) {
        total_vrefint += adcx_get_chx_value(&hadc1, ADC_CHANNEL_VREFINT);
    }

    ADCVrefintProportion = 200 * 1.200f / total_vrefint;
}

/**
  * @brief          ADC�ɼ�STM32�ڲ��¶�
  * @retval         none
  */
float ADC_Get_STM32Temprate(void)
{
	float temperate = 0.000f;
    uint16_t adcx = 0;

    adcx = adcx_get_chx_value(&hadc1, ADC_CHANNEL_TEMPSENSOR);
    temperate = (float)adcx * ADCVrefintProportion;
    temperate = (temperate - 0.76f) * 400.0f + 25.0f;

    return temperate;
}

/**
  * @brief          ADC�ɼ���ص�ѹ
  * @retval         none
  */
float ADC_Get_Voltage(void)
{
    float voltage = 0.000f;
    uint16_t adcx = 0;

    adcx = adcx_get_chx_value(&hadc3, ADC_CHANNEL_8);
    //(22K �� + 200K ��)  / 22K �� = 10.090909090909090909090909090909
    voltage =  (float)adcx * ADCVrefintProportion * 10.090909090909090909090909090909f;

    return voltage;
}
