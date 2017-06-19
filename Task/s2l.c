#include "s2l.h"
#include "cmsis_os.h"
#include "stm32l0xx_hal.h"
#include "fhex.h"
#include "bsp.h"
#include "string.h"
#include "event.h"

extern osMessageQId UartQHandle;
extern UART_HandleTypeDef huart1;





/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
uint16_t const crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

uint16_t crc16(uint8_t data,uint16_t crc)
{
  return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}

typedef enum
{
  IDLE=0U,
  SOF_1,
  SOF_2,
  LEN_1,
  LEN_2,
  CLEN_1,
  DATA,
  DONE
}FrameStateDef;

typedef struct
{
  FrameStateDef state;
  uint8_t* pdata;
}CmdTypeDef;

//static uint8_t rx;
static CmdTypeDef cmd;
static void CmdTimeout()
{
  //TODO:maybe error
  if(cmd.state==SOF_1)
    return;
  if(cmd.state>=DATA)
  {
    vPortFree(cmd.pdata);
  }
  if(cmd.state!=IDLE)
  {
    //HAL_UART_Transmit(&huart1,&cmd.state,1,200);
    cmd.state=IDLE;
  }
  TimeStop();
}
void S2lTask()
{
  osEvent evt;
  cmd.state=IDLE;
  
  uint16_t length,clen,crcseed;
  uint16_t tmpl; 
  //HAL_UART_Receive_IT(&huart1,&rx,1);
//  huart1.RxState=HAL_UART_STATE_BUSY_RX;
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  while(1)
  {
    evt=osMessageGet(UartQHandle,osWaitForever);
    
    
    
    uint8_t tmp;
    tmp=(uint8_t)evt.value.v;
    switch(cmd.state)
    {
    case IDLE:
      {
        if(tmp==0x55)
        {
          cmd.state=SOF_1;
          //osTimerStart(timeoutHandle,200);
          TimeStart(CmdTimeout,200);
        }
        else
        {
          cmd.state=IDLE;
        }
        break;
      }
    case SOF_1:
      {
        if(tmp==0xAA)
        {
          cmd.state=SOF_2;
          
        }
        else
        {
          cmd.state=IDLE;
          
        }
        break;
      }
    case SOF_2:
      {
        length=((uint16_t)tmp)<<8&0xff00;
        cmd.state=LEN_1;
        break;
      }
    case LEN_1:
      {
        length|=tmp;
        cmd.state=LEN_2;
        break;
      }
    case LEN_2:
      {
        clen=((uint16_t)tmp)<<8&0xff00;
        cmd.state=CLEN_1;
        break;
      }
    case CLEN_1:
      {
        clen|=tmp;
        clen=~clen;
        if(length!=clen)
        {
          cmd.state=IDLE;
          break;
        }
        tmpl=length;
        //data
        cmd.pdata=pvPortMalloc(length+7);
        cmd.pdata[0]=0x55;
        cmd.pdata[1]=0xaa;
        cmd.pdata[2]=(uint8_t)(length>>8);
        cmd.pdata[3]=(uint8_t)(length&0x00ff);
        cmd.pdata[4]=(uint8_t)(clen>>8);
        cmd.pdata[5]=(uint8_t)(clen&0x00ff);
        cmd.state=DATA;
        break;
      }
    case DATA:
      {
        tmpl--;
        cmd.pdata[5+length-tmpl]=tmp;
        if(tmpl==0)
        {
          cmd.state=DONE;
        }
        break;
      }
    case DONE:
      {
        //osTimerStop(timeoutHandle);
        TimeStop();
        cmd.state=IDLE;
        cmd.pdata[length+6]=0;
        crcseed=0xffff;
        for(int i=0;i<length-2;i++)
        {
          crcseed=crc16(cmd.pdata[i+6],crcseed);
        }
        if(crcseed==(cmd.pdata[length+4]<<8|cmd.pdata[length+5]))
        {
          //success
          //SendAck();
          //
           uint16_t id;
          id=cmd.pdata[6]<<8|cmd.pdata[7];
          ExecCmd(id,cmd.pdata);
        }
        vPortFree(cmd.pdata);
        break;
      }
      
      
    }
  }
  
}




void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//  if(huart->Instance ==USART1)
//  {
//    //
//    osMessagePut (UartQHandle,(uint32_t)rx,0);//no block in the inter
//  }
}

void USART1_IRQHandler(void)
{
  
  if((__HAL_UART_GET_IT(&huart1, UART_IT_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE)))
  {
    uint8_t tmp;
    tmp=READ_REG(huart1.Instance->RDR);
    osMessagePut (UartQHandle,(uint32_t)tmp,0);//no block in the inter
  }
  
  if(__HAL_UART_GET_IT(&huart1,UART_IT_ORE))
    __HAL_UART_CLEAR_OREFLAG(&huart1);
  //vPortEnterCritical();
  //HAL_UART_IRQHandler(&huart1);
  //vPortExitCritical();
  //HAL_UART_Receive_IT(&huart1,&rx,1);
}