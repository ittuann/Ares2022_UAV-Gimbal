/*
 * ist8310driver_middleWare.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "ist8310driver_middleWare.h"

extern I2C_HandleTypeDef hi2c3;

/**
  * @brief          ��ȡIST8310��һ���ֽ�ͨ��I2C
  * @param[in]      �Ĵ�����ַ
  * @retval         �Ĵ���ֵ
  */
uint8_t ist8310_IIC_read_single_reg(uint8_t reg)
{
    uint8_t res = 0;
    // I2C��� I2C�ӻ���ַ �Ĵ�����ַ �Ĵ�����ַ���Ӵ�С ����ָ�� ���ݳ��� ��ʱʱ��
    HAL_I2C_Mem_Read(&hi2c3, IST8310_IIC_ADDRESS <<1, reg, I2C_MEMADD_SIZE_8BIT, &res, 1, 2);	// ��I2C�豸�ļĴ�����ȡ����
    return res;
}

/**
  * @brief          ͨ��I2Cд��һ���ֽڵ�IST8310�ļĴ�����
  * @param[in]      �Ĵ�����ַ
  * @param[in]      д��ֵ
  * @retval         none
  */
void ist8310_IIC_write_single_reg(uint8_t reg, uint8_t data)
{
	// I2C��� I2C�ӻ���ַ �Ĵ�����ַ �Ĵ�����ַ���Ӵ�С ����ָ�� ���ݳ��� ��ʱʱ��
    HAL_I2C_Mem_Write(&hi2c3, IST8310_IIC_ADDRESS <<1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 2);	// ��I2C�豸�ļĴ���д������
}

/**
  * @brief          ��ȡIST8310�Ķ���ֽ�ͨ��I2C
  * @param[in]      �Ĵ�����ʼ��ַ
  * @param[out]     ��ȡ������
  * @param[in]      ��ȡ�ֽ�����
  * @retval         none
  */
void ist8310_IIC_read_muli_reg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c3, IST8310_IIC_ADDRESS <<1, reg, I2C_MEMADD_SIZE_8BIT, buf, len, 10);
}

/**
  * @brief          д�����ֽڵ�IST8310�ļĴ���ͨ��I2C
  * @param[in]      �Ĵ�����ʼ��ַ
  * @param[out]     ��ȡ������
  * @param[in]      ��ȡ�ֽ�����
  * @retval         none
  */
void ist8310_IIC_write_muli_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_I2C_Mem_Write(&hi2c3, IST8310_IIC_ADDRESS <<1, reg,I2C_MEMADD_SIZE_8BIT, data, len, 10);
}
