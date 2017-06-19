#ifndef FHEX_H
#define FHEX_H

#include "stm32l0xx_hal.h"


typedef struct
{
  uint16_t cmdid;
  void(*exec)(uint8_t* data);
}FuncTypeDef;

void SendAck();
void ExecCmd(uint16_t id,uint8_t* pdata);
void S2L_LOG(uint8_t* log);


#endif