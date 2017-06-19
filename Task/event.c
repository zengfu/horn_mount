#include "stm32l0xx_hal.h"
#include "cmsis_os.h"
#include "event.h"
#include "board.h"
#include "lis3dx.h"
#include "lte.h"
#include "s2l.h"
#include "fhex.h"

extern osMessageQId EventQHandle;
extern osThreadId lteHandle;
extern CarTypeDef car;

extern osMutexId EventLockHandle;

void PowerEventHandle(EventTypeDef *event);
void AccelEventHandle(EventTypeDef *event);
void LteEventHandle(EventTypeDef *event);
void MicroWaveEventHandle(EventTypeDef *event);
void SimEventHandle(EventTypeDef *event);


uint16_t GlobalEvent=0x8000;
void EventTask()
{
  EventTypeDef event;
  osDelay(1000);
  PowerS2l(1);
  while(1)
  {
    
    xQueueReceive(EventQHandle,&event,portMAX_DELAY);
   
    if(event.evt==EvtAccel1)
    {
      //AccelEventHandle(&event);
    }
    else if(event.evt==EvtMicroWave)
    {
      MicroWaveEventHandle(&event);
    }
    else if(event.evt==EvtLte)
    {
      LteEventHandle(&event);
    }
    else if(event.evt==EvtSimCard)
    {
      SimEventHandle(&event);
    }
    //TODO:
  }
}
void SimEventHandle(EventTypeDef *event)
{
  //SetEvent(UPLOAD_EVENT_CARD,1);
  if(event->io==GPIO_PIN_RESET)
  {
    S2L_LOG("card pull out\n");
    car.hardcard=0;
  }
  else
  {
    S2L_LOG("card pull in\n");
    car.hardcard=1;
  }
}
void MicroWaveEventHandle(EventTypeDef *event)
{
  static uint32_t PressTime,RealeaseTime;
  if(event->io==GPIO_PIN_RESET)
  {
    PressTime=event->tick;
    //PirLevelSet(cnt%4);
    LedSet(0);
    LogoCtrl(0);
    S2L_LOG("MicroWave down\r\n");
    //cnt++;
  }
  else
  {
    RealeaseTime=event->tick;
//    osMutexWait(EventLockHandle,osWaitForever);
//    GlobalEvent|=UPLOAD_EVENT_MW;
//    osMutexRelease(EventLockHandle);
//    PowerS2l(1);
    SetEvent(UPLOAD_EVENT_MW,1);
    S2L_LOG("MicroWave\r\n");
    LedSet(1);
    LogoCtrl(1);
    //printf("wm:%d\r\n",(RealeaseTime-PressTime));
  }
}

void LteEventHandle(EventTypeDef *event)
{
  //check the LteTask status;
  
  if(osThreadIsSuspended(lteHandle)==osOK)
  {
    //LedTog(2);
    uint8_t state=0;
    //HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
    state=CheckFrame();
    //osDelay(5000);
    //HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
    if((state&LTE_WAKEUP))
    {
//      osMutexWait(EventLockHandle,osWaitForever);
//      GlobalEvent|=UPLOAD_EVENT_LTE;
//      osMutexRelease(EventLockHandle);
//      PowerS2l(1);
      SetEvent(UPLOAD_EVENT_LTE,1);
      SCMWakeup();
      //wake up thing
      //LedSet(2,1);
      S2L_LOG("wakeup\r\n");
      //osDelay(100);
      //LedSet(2,0);
      //todo:
    }
  }
}
void AccelEventHandle(EventTypeDef *event)
{
  static uint32_t PressTime,RealeaseTime;
  if(event->io==GPIO_PIN_SET)
  {
    PressTime=event->tick;
    //LedTog(1);
    //PirLevelSet(cnt%4);
    //cnt++;
//    osMutexWait(EventLockHandle,osWaitForever);
//    GlobalEvent|=UPLOAD_EVENT_ACCEL;
//    osMutexRelease(EventLockHandle);
//    PowerS2l(1);
    SetEvent(UPLOAD_EVENT_ACCEL,1);
    S2L_LOG("accel\r\n");
  }
  else
  {
    RealeaseTime=event->tick;
  }
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin==AC_IT1_PIN)
  {
    EventTypeDef tmpevt;
    tmpevt.evt=EvtAccel1;
    tmpevt.tick=HAL_GetTick();
    tmpevt.io=HAL_GPIO_ReadPin(AC_IT1,AC_IT1_PIN);
    //osMessagePut(dangerHandle,*(uint32_t *)&tmpevt,0);
    xQueueSendFromISR(EventQHandle,&tmpevt,0);
  }
  if(GPIO_Pin==LTE_IT_PIN)
  {
    EventTypeDef tmpevt;
    tmpevt.evt=EvtLte;
    tmpevt.tick=HAL_GetTick();
    //LedTog(1);
    if(!HAL_GPIO_ReadPin(LTE_IT,LTE_IT_PIN))
    {
      tmpevt.io=GPIO_PIN_RESET;
    //osMessagePut(dangerHandle,*(uint32_t *)&tmpevt,0);
      xQueueSendFromISR(EventQHandle,&tmpevt,0);
    }
  }
  if(GPIO_Pin==MW_IT_PIN)
  {
  
    {
      EventTypeDef tmpevt;
      tmpevt.evt=EvtMicroWave;
      tmpevt.tick=HAL_GetTick();
      tmpevt.io=HAL_GPIO_ReadPin(MW_IT,MW_IT_PIN);
      //osMessagePut(dangerHandle,*(uint32_t *)&tmpevt,0);
      xQueueSendFromISR(EventQHandle,&tmpevt,0);
    }
  }
  if(GPIO_Pin==SIM_IT_PIN)
  {
  
    {
      EventTypeDef tmpevt;
      tmpevt.evt=EvtSimCard;
      tmpevt.tick=HAL_GetTick();
      tmpevt.io=HAL_GPIO_ReadPin(SIM_IT,SIM_IT_PIN);
      xQueueSendFromISR(EventQHandle,&tmpevt,0);
    }
  }
}
void SetEvent(uint8_t event,uint8_t on_off)
{
    osMutexWait(EventLockHandle,osWaitForever);
    GlobalEvent|=event;
    PowerS2l(on_off);
    osMutexRelease(EventLockHandle);
    
}