#include "stm32l0xx_hal.h"
#include "cmsis_os.h"
#include "event.h"
#include "board.h"
#include "lis3dx.h"
#include "lte.h"
#include "s2l.h"
#include "fhex.h"
#include "period.h"
#include "arm_math.h"

extern ADC_HandleTypeDef hadc;
//period is 50ms
uint8_t pin;
uint32_t adc[3];
void PeriodTask()
{
  uint8_t mw_cnt=0;
  uint8_t adc_cnt=0;
  while(1)
  {
    //10s
    if(mw_cnt==20)
    {
      mw_cnt=0;
      if(HAL_GPIO_ReadPin(MW_IT,MW_IT_PIN))
      {
        S2L_LOG("radio\n");
        //SetEvent(UPLOAD_EVENT_MW,1);
      }
    }
    //1s
    if(adc_cnt==2)
    {
      adc_cnt=0;
      AdcCtrl(1);
      HAL_ADC_PollForConversion(&hadc,10);
      adc[0]=HAL_ADC_GetValue(&hadc);
      HAL_ADC_PollForConversion(&hadc,10);
      adc[1]=HAL_ADC_GetValue(&hadc);
      HAL_ADC_PollForConversion(&hadc,10);
      adc[2]=HAL_ADC_GetValue(&hadc);
      AdcCtrl(0);
      car.voltage=ComputeVoltage(adc[0],adc[1]);
      car.temp=ComputeTemperature(adc[2]);
      if((car.voltage>14)!=car.engine)
      {
        car.engine=(car.voltage>14);
        //SetEvent(UPLOAD_EVENT_ENGINE,1);
        S2L_LOG("engine\n");
      }
    }
    adc_cnt++;
    mw_cnt++;
    //LedTog();
    osDelay(500);
    //adc=GetVoltage();
  }
}
