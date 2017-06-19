#ifndef BASE64_h
#define BASE64_h

#include "stm32l0xx_hal.h"

uint8_t* b64_encode (uint8_t *in);
uint8_t* b64_decode (uint8_t *in);

#endif