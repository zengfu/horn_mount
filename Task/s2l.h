#ifndef S2L_h
#define S2L_H

#include "stm32l0xx_hal.h"
#include "bsp.h"



void S2lTask();
uint16_t crc16(uint8_t data,uint16_t crc);






#endif