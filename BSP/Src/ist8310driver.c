/*
 * ist8310driver.c
 *
 *  Created on: 2021��10��24��
 *      Author: LBQ
 */
#include "ist8310driver.h"

#define IST8310_WHO_AM_I		0x00	// ist8310 "who am I "
#define IST8310_WHO_AM_I_VALUE	0x10	// device ID

#define IST8310_DATA_READY_BIT	2
#define IST8310_WRITE_REG_NUM	4

#define MAG_SEN					0.3f	// ԭʼ��������ת���� ��λut

// ����RSTN����Ϊ1
void ist8310_RST_H(void)
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
}
// ����RSTN����Ϊ0
void ist8310_RST_L(void)
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
}

// ��һ��:IST8310�ļĴ���
// �ڶ���:��Ҫд��ļĴ���ֵ
const uint8_t ist8310_write_reg_data[IST8310_WRITE_REG_NUM][2] = {
        {0x0B, 0x08},	// �жϼĴ���, �����ж�, ��������DRDY�ܽ��ж�ʱΪ�͵�ƽ
//		{0x41, 0x09},	// ���������Ĵ���, ƽ����������
		{0x41, 0x12},	// ƽ������4�� 8��Ϊ0x1b 16��Ϊ0x24
        {0x42, 0xC0},	// ������0xC0
        {0x0A, 0x0B}	// ��������ģʽ���Ƶ��200Hz
};

void ist8310_init(void)
{
	const uint8_t wait_time = 150;
	const uint8_t sleepTime = 50;
    uint8_t res = 0;
    uint8_t writeNum = 0;

    // ͨ��IST8310�����ܽŽ�������
    ist8310_RST_L();
    HAL_Delay(sleepTime);
    ist8310_RST_H();
    HAL_Delay(sleepTime);

    // ȡID�Ĵ���
    res = ist8310_IIC_read_single_reg(IST8310_WHO_AM_I);
    // ȷ��ID��ȷ �ж�ͨ���Ƿ�����
    if (res != IST8310_WHO_AM_I_VALUE)
    {

    }

    // ����IST8310����״̬
    for (writeNum = 0; writeNum < IST8310_WRITE_REG_NUM; writeNum ++ ) {
        ist8310_IIC_write_single_reg(ist8310_write_reg_data[writeNum][0], ist8310_write_reg_data[writeNum][1]);
        Delay_us(wait_time);
        res = ist8310_IIC_read_single_reg(ist8310_write_reg_data[writeNum][0]);
        Delay_us(wait_time);
        // У��
        if (res != ist8310_write_reg_data[writeNum][1])
        {

        }
    }
}

/**
  * @brief          ����Ѿ�ͨ��I2C��DMA��ʽ��ȡ���˴�STAT1��DATAXL��7�����ݣ�����ʹ������������д���ɵ�λ��uT�Ĵų�ǿ������
  * @param[in]      status_buf:����ָ��,��STAT1(0x02) �Ĵ����� DATAXL(0x08)�Ĵ���
  * @param[out]
  * @retval         none
  */
void ist8310_read_over(uint8_t *status_buf, fp32 mag[3], uint8_t magStatus)
{
	// ͨ���ж�stat1�Ĵ���ֵ�ж���û���µ����ݲ���
    if (status_buf[0] & 0x01) {
        int16_t temp_ist8310_data = 0;
        magStatus |= 1 << IST8310_DATA_READY_BIT;

        temp_ist8310_data = (int16_t)((status_buf[2] << 8) | status_buf[1]);
        mag[0] = MAG_SEN * temp_ist8310_data;
        temp_ist8310_data = (int16_t)((status_buf[4] << 8) | status_buf[3]);
        mag[1] = MAG_SEN * temp_ist8310_data;
        temp_ist8310_data = (int16_t)((status_buf[6] << 8) | status_buf[5]);
        mag[2] = MAG_SEN * temp_ist8310_data;
    }
    else {
    	magStatus &= ~(1 << IST8310_DATA_READY_BIT);
    }
}

/**
  * @brief          ͨ����ȡ�ų�����
  * @param[out]     �ų�����
  * @retval         none
  */
void ist8310_read_mag(fp32 mag[3])
{
    uint8_t buf[6] = {0};
    int16_t temp_ist8310_data = 0;
    // I2C��ȡ����ֽ�
    ist8310_IIC_read_muli_reg(0x03, buf, 6);

    temp_ist8310_data = (int16_t)((buf[1] << 8) | buf[0]);
    mag[0] = MAG_SEN * temp_ist8310_data;
    temp_ist8310_data = (int16_t)((buf[3] << 8) | buf[2]);
    mag[1] = MAG_SEN * temp_ist8310_data;
    temp_ist8310_data = (int16_t)((buf[5] << 8) | buf[4]);
    mag[2] = MAG_SEN * temp_ist8310_data;
}
