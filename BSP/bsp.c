#include "bsp.h"
#include "lis3dx.h"
#include "s2l.h"
#include "string.h"


#define TSENSE_CAL2 ((uint16_t*) ((uint32_t) 0x1FF8007E))
#define TSENSE_CAL1 ((uint16_t*) ((uint32_t) 0x1FF8007A))
#define VREFINT_CAL ((uint16_t*) ((uint32_t) 0x1FF80078))
#define VDD_CALIB ((uint16_t) (330))
#define VDD_APPLI ((uint16_t) (330))

#define  NVM_CAR_STATUS DATA_EEPROM_BASE+0



extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc;
extern UART_HandleTypeDef hlpuart1;


const uint8_t pwm_table[] = {1,1,2,2,3,4,6,8,10,14,19,25,33,44,59,80,
    107,143,191,255,255,191,143,107,80,59,44,33,25,19,14,10,8,6,4,3,2,2,1,1,1,1,1,1,1,1};


static void LteInit();
static void (*TimeCallback)();
static void CheckHardCard();
static void ExtiCtrl(uint8_t cmd);





CarTypeDef car;
CameraTypeDef camera;


void BspInit()
{
  LteInit();
  LteCtrl(0);
  memset(&car,0,sizeof(car));
  //car.reserve=0x55;
  //SaveCarStatus();
  memset(&car,0,sizeof(car));
  LoadCarStatus();
  //car.front_ir=1;
  car.reserve=0x55;
  //car.mw=1;
  //BreathCtrl(1);
  CheckHardCard();
  ExtiCtrl(0);
  Lis3dxInit();
  //afterir todo
  //LteCtrl(car.lte);
  GIrCtrl(car.front_ir);
  MwCtrl(car.mw);
  LogoCtrl(car.logo);
  GpsCtrl(car.gps);
  //active 
  //plan
  ExtiCtrl(1);
  BreathCtrl(1);
#ifdef LTE_ZTE
  LteInit();
  //PIrCtrl(1);
#endif
}

static void ExtiCtrl(uint8_t cmd)
{
  if(cmd){
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  }
  else{
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
  }
  
}
void TimeStart(void (*func)(),uint16_t time)
{
  TimeSetTimeout(time);
  HAL_TIM_Base_Start_IT(&htim6);
  TimeCallback=func;
}
void TimeStop()
{
  HAL_TIM_Base_Stop_IT(&htim6);
  TimeCallback=NULL;
}
//ms
void TimeSetTimeout(uint16_t period)
{
  htim6.Instance->ARR=period;
}

static void CheckHardCard()
{
  if(HAL_GPIO_ReadPin(SIM_IT,SIM_IT_PIN))
    car.hardcard=1;
  else
    car.hardcard=0;
}
void GpsCtrl(uint8_t cmd)
{
  car.gps=cmd;
  HAL_GPIO_WritePin(GPS_EN_PORT,GPS_EN,(GPIO_PinState)cmd);
}
void AdcCtrl(uint8_t cmd)
{
  if(cmd)
  {
    HAL_ADC_Start(&hadc);
  }
  else
    HAL_ADC_Stop(&hadc);
}
void LogoCtrl(uint8_t cmd)
{
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
  if(cmd)
  {
    car.logo=1;
    htim2.Instance->CCR2=999;
    
  }
  else
  {
    car.logo=0;
    htim2.Instance->CCR2=0;
  }
}
void BreathCtrl(uint8_t cmd)
{
  if(cmd)
  {
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
  }
  else
  {
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_UPDATE);
    HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_2);
  }
}




int32_t ComputeTemperature(uint32_t measure)
{
  int32_t temperature;
  temperature = ((measure * VDD_APPLI / VDD_CALIB)- (int32_t) *TSENSE_CAL1 );
  temperature = temperature *(int32_t)(130-30);
  temperature = temperature / (int32_t)(*TSENSE_CAL2 -
    *TSENSE_CAL1);
  temperature = temperature + 30;
  return(temperature);
}
float ComputeVoltage(uint32_t measure,uint32_t vref)
{
  return 3.0*(*VREFINT_CAL)*measure/((float)vref)/4095.0*21;
}
                  
//void GetVoltageTemp(float *voltage,float* temp)
//{
//  sample_flag=0;
//  AdcCtrl(1);
//  while(sample_flag==0)
//  {
//    osDelay(10);
//  }
//  *voltage=ComputeAdc(adc[0],adc[1]);
//  //temp=(100.0/(TSENSE_CAL2-TSENSE_CAL1))*(adc[2]-TSENSE_CAL1)+30.0;
//  *temp=ComputeTemperature(adc[2]);
//  AdcCtrl(0);
//}
//
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//  static uint8_t cnt =0;
//  adc[cnt++]=HAL_ADC_GetValue(hadc);
//  if(cnt==3)
//  {
//    sample_flag=1;  
//    cnt=0;
//  }
//    
//}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM6)
  {
    if(TimeCallback)
    TimeCallback();
  }
  if(htim->Instance==TIM2)
  {
    static uint8_t cnt=0;
    static uint8_t pwm_index=0;
    if(cnt==50)
    {
      cnt=0;
      htim2.Instance->CCR2=pwm_table[pwm_index]*3;
      pwm_index++;
      if(pwm_index==sizeof(pwm_table))
        pwm_index=0;
    }
    cnt++;
  }
  
}


void MwCtrl(uint8_t on)
{
  HAL_GPIO_WritePin(MV_EN_PORT,MV_EN,(GPIO_PinState)on);
}
void LedTog()
{
  HAL_GPIO_TogglePin(LED1_PORT,LED1);  
}
void LedSet(uint8_t state)
{
  HAL_GPIO_WritePin(LED1_PORT,LED1,(GPIO_PinState)state);
}

//void LteStop()
//{
//  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_SET);//OPEN
//}
static void LteInit()
{
#ifdef LTE_ZTE
  HAL_GPIO_WritePin(LTE_RESET_PORT,LTE_RESET,GPIO_PIN_SET);//NO RESET
  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_RESET);//close
#else
  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_RESET);//close
  HAL_GPIO_WritePin(LTE_WAKEUP_PORT,LTE_WAKEUP_PIN,GPIO_PIN_SET);//AUTO SLEEP
  HAL_GPIO_WritePin(LTE_RESET_PORT,LTE_RESET,GPIO_PIN_SET);//NO RESET
  //HAL_Delay(1000);
//  for(int i=0;i<500000000;i++){
//  
//  }
//  HAL_GPIO_WritePin(LTE_WAKEUP_PORT,LTE_WAKEUP_PIN,GPIO_PIN_RESET);//AUTO SLEEP
#endif
  
}
void LteCtrl(uint8_t cmd)
{
  if(cmd){
     HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_RESET);
     car.lte=1;
  }
  else{
    car.lte=0;
    HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_SET);
  }
  
}
void LteReset()
{
  HAL_GPIO_WritePin(LTE_RESET_PORT,LTE_RESET,GPIO_PIN_RESET);//NO RESET
  osDelay(2000);
  HAL_GPIO_WritePin(LTE_RESET_PORT,LTE_RESET,GPIO_PIN_SET);//NO RESET
}
void LteRestart()
{
  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_SET);//close
  osDelay(1000);
  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_RESET);//close
}
void PowerS2l(uint8_t in)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(in==0)
  {
    
    GPIO_InitStruct.Pin = P_IR_EN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(P_IR_EN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(P_IR_EN_PORT,P_IR_EN,GPIO_PIN_RESET);//close
  }
  else
  {
    GPIO_InitStruct.Pin = P_IR_EN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(P_IR_EN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(P_IR_EN_PORT,P_IR_EN,GPIO_PIN_RESET);//close
    osDelay(100);
//    GPIO_InitStruct.Pin = P_IR_EN;
//    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(P_IR_EN_PORT, &GPIO_InitStruct);
    //HAL_GPIO_WritePin(P_IR_EN_PORT,P_IR_EN,GPIO_PIN_RESET);//close
    HAL_GPIO_WritePin(P_IR_EN_PORT,P_IR_EN,GPIO_PIN_SET);//close
  
  }
}
void PIrCtrl(uint8_t cmd)
{
  //HAL_GPIO_WritePin(P_IR_EN_PORT,P_IR_EN,(GPIO_PinState)cmd);//close
  HAL_GPIO_WritePin(ON_OFF_PORT,ON_OFF,(GPIO_PinState)cmd);//close
}

void GIrCtrl(uint8_t cmd)
{
  HAL_GPIO_WritePin(G_IR_EN_PORT,G_IR_EN,(GPIO_PinState)cmd);//close
}

void LteOpen()
{
  HAL_GPIO_WritePin(LTE_CLOSE_PORT,LTE_CLOSE,GPIO_PIN_RESET);
}

void SaveCarStatus()
{
  {
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_HALFWORD, NVM_CAR_STATUS,*((uint16_t*)&car));
    HAL_FLASHEx_DATAEEPROM_Lock();
  }
}
void LoadCarStatus()
{
  uint16_t tmp;
  tmp=*(__IO uint16_t *)NVM_CAR_STATUS;
  memcpy((uint8_t*)(uint8_t*)&car,&tmp,2);
}

/*
extern UART_HandleTypeDef huart1;
int fputc(int ch, FILE *f)
{

 

  uint8_t a;

  a=(uint8_t)ch;

  HAL_UART_Transmit(&huart1, &a, 1, 100);

  return ch;

}*/