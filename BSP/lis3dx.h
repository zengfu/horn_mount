#ifndef LIS3DX_H
#define LIS3DX_H

#include "stm32l0xx_hal.h"

uint8_t Lis3dxInit();

typedef struct
{
  uint8_t upload;
  uint8_t odr;    //output data rate
  uint8_t scale;  // +-2 +-3 +-8 +-16
  uint8_t ths;     // lsb 16 32 63 125
}Lis3dxTypeDef;


/****CTRL_REG1*****/
#define ODR_1HZ 0X10
#define ODR_10HZ 0X20
#define ODR_25HZ 0X30
#define ODR_50HZ 0X40
#define ODR_100HZ 0X50
#define ODR_200HZ 0X60
#define ODR_400HZ 0X70
#define ODR_1600HZ 0X80
#define ODR_5000HZ 0X90

#define NORMAL_MODE 0X00
#define LOWPWR_MODE 0x08

#define ZAXI_ENABLE 0X04
#define YAXI_ENABLE 0X02
#define XAXI_ENABLE 0X01

/****CTRL_REG2*****/
#define HPM_ENABLE 0x08


/****CTRL_REG3*****/
#define INT_AOI1 0X40

/****CTRL_REG4*****/
#define SCALE_2G 0x00
#define SCALE_4G 0X10
#define SCALE_8G 0X20
#define SCALE_16 0X30
#define HR_ENABLE 0X08

/****CTRL_REG5*****/


/****CTRL_REG6*****/


void Lis3dhThs(uint8_t level);
void Lis3dhConfig(uint8_t odr,uint8_t scale);
void accel_read(int16_t* accel);
void accel_raw_read(uint8_t* raw);
uint8_t ReadIntCnt();

#endif