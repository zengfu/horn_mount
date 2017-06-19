#ifndef BSP_H
#define BSP_H

#include "board.h"
#include "stm32l0xx_hal.h"
#include "cmsis_os.h"



typedef struct
{
  //rw
  uint16_t gps:1;//LSB
  uint16_t mw:1;
  uint16_t plan:1;
  uint16_t active:1;
  uint16_t logo:1;
  uint16_t front_ir:1;
  uint16_t after_ir:1;
  uint16_t lte:1;
  uint16_t reserve:8;
  //read only
  uint16_t engine:1; 
  uint16_t card:1;
  uint16_t hardcard:1;
  uint16_t token:1;
  uint16_t :0;
  //
  float voltage;
  int32_t temp;
  
}CarTypeDef;



void ReInitLuart();
void LteOpen();
void PowerS2l(uint8_t in);
void LedSet(uint8_t num);
void BspInit();
void PirLevelSet(uint8_t x);
void LedTog();
void LteStop();
void LteRestart();
void TimeStop();
void TimeStart(void (*func)(),uint16_t time);
void TimeSetTimeout(uint16_t period);
void LteReset();
void CheckActive();
void DeActiveDevice();
void ActiveDevice();
void MwCtrl(uint8_t on);
void BreathCtrl(uint8_t cmd);
void GpsCtrl(uint8_t cmd);
void AdcCtrl(uint8_t cmd);
void ReadCalib();
void LogoCtrl(uint8_t cmd);
int32_t ComputeTemperature(uint32_t measure);
float ComputeVoltage(uint32_t measure,uint32_t vref);
void LteCtrl(uint8_t cmd);
void PIrCtrl(uint8_t cmd);
void GIrCtrl(uint8_t cmd);
void LoadCarStatus();
void SaveCarStatus();

extern CarTypeDef car;
 


#endif