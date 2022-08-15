/*
 * BMI088_Middleware.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "BMI088_Middleware.h"

extern SPI_HandleTypeDef hspi1;

float IMU_Gyro[3] = {0}, IMU_Accel[3] = {0};
float IMU_Temperate = 0;

/**
 * @brief			ͨ��SPI���������ʹӻ���ͨ��
 * @param[in]		tx_data : ����������
 * @return			rx_data : ��������
 */
uint8_t BMI088_read_write_byte(uint8_t tx_data)
{
    uint8_t rx_data;
    HAL_SPI_TransmitReceive(&hspi1, &tx_data, &rx_data, 1, 1);	// ͨ��SPI���������ʹӻ���ͨ��
    return rx_data;
}

/**
 * @brief			SPI����д��
 * @param[in]		reg : �Ĵ�����ַ
 * @param[in]		data : ָ��
 */
void BMI088_write_single_reg(uint8_t reg, uint8_t data)
{
    BMI088_read_write_byte(reg);
    BMI088_read_write_byte(data);
}
/**
 * @brief			SPI���ζ�ȡ
 * @param[in]		reg : �Ĵ�����ַ
 * @param[out]		return_data : ��������
 */
void BMI088_read_single_reg(uint8_t reg, uint8_t *return_data)
{
    BMI088_read_write_byte(reg | 0x80);
    *return_data = BMI088_read_write_byte(0x55);
}

void BMI088_write_muli_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    BMI088_read_write_byte(reg);
    while( len != 0 ) {
        BMI088_read_write_byte(*data);
        data ++ ;
        len -- ;
    }
}
void BMI088_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    BMI088_read_write_byte(reg | 0x80);
    while (len != 0) {
        *buf = BMI088_read_write_byte(0x55);
        buf ++ ;
        len -- ;
    }
}

/**
 * @brief			������SPI��ʼ��
 */
void BMI088_init(void)
{
	bmi088_accel_init();
	bmi088_gyro_init();
}
