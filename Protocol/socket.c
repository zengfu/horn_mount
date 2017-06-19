#include "cmsis_os.h"
#include "socket.h"
#include "bsp.h"
#include "lte.h"
#include "string.h"
#include "fhex.h"

#define LTE_UART hlpuart1

#define nop_time 500 

extern LteTypeDef lte;
extern UART_HandleTypeDef hlpuart1;

static uint8_t* read();

const char* ltecmd[]=
{
  "AT\r\n",
  "AT%STATUS=\"USIM\"\r\n",
  "AT%MEAS=\"0\"\r\n",
  "AT^SISC=0\r\n",//socket close
  "AT^SISS=0,srvType,STREAM\r\n",//tcp
  "AT^SISS=0,address,tscastle.cam2cloud.com\r\n",//ip address
  "AT^SISS=0,port,36003\r\n",//ip port
  //"AT^SISS=0,address,www.bananalife.top\r\n",//ip address
  //"AT^SISS=0,port,7014\r\n",//ip port
  "AT^SICI=0\r\n",//connect init
  "AT^SISO=0\r\n",//socket open  //NOTE:the cmd need refresh
  "AT^SISHR=0\r\n",
    
};

static uint8_t idle_done=0;
void AES_RNG_LPUART1_IRQHandler(void)
{
    if(__HAL_UART_GET_IT(&hlpuart1,UART_IT_RXNE))
    {
      lte.rx_buf[lte.length]=READ_REG(hlpuart1.Instance->RDR);
      lte.length++;
    }
    
    if(__HAL_UART_GET_IT(&hlpuart1,UART_IT_IDLE))
    {
      __HAL_UART_CLEAR_IDLEFLAG(&hlpuart1);
      lte.rx_buf[lte.length]='\0';
      idle_done=1;
      lte.length++;   
    }
    
    if(__HAL_UART_GET_IT(&hlpuart1,UART_IT_ORE))
    __HAL_UART_CLEAR_OREFLAG(&hlpuart1);
}
static uint8_t check(uint8_t id,uint8_t*data)
{
  if(data==NULL)
  {
    return 2;
  }
  else
  {
    //uint16_t len=strlen(data);
    if(lte.length==0)
      return 3;
    switch(id)
    {
    case 0:
      {
        for(int i=0;i<lte.length;i++)
        {
          if(data[i]=='O'&&data[i+1]=='K')
          {
            return 0;
          }
        }
        return 1;
      }
    case 1:
      {
        for(int i=0;i<lte.length;i++)
        {
          if(data[i]=='R'&&data[i+1]=='E'&&data[i+2]=='A'&&data[i+3]=='L')
          {
            return 0;
          }
        }
        return 1;
      }
    case 2:
      {
        for(int i=0;i<lte.length;i++)
        {
          if(data[i]=='S'&&data[i+1]=='I'&&data[i+2]=='S'&&data[i+3]=='H'&&data[i+3]=='R'&&data[i+4]==':')
          {
            return 0;
          }
        }
        return 1;
      }
    }
   
  }
  return 1;
}
static void SendCmd(uint8_t * cmd)
{
  uint8_t count=0;
  idle_done=0;
  lte.length=0;
  __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_RXNE);

  HAL_UART_Transmit(&LTE_UART,cmd,strlen((const char*)cmd),200);
  while((!idle_done))
  {
    osDelay(50);
    if(lte.length==0)
    {
      count++;
      //HAL_UART_Transmit(&LTE_UART,cmd,strlen(cmd),200);
    }
    if(count==10)
      break;
  }
  __HAL_UART_DISABLE_IT(&hlpuart1, UART_IT_RXNE);
}
uint8_t CheckCard()
{
  uint8_t *tmp="AT%STATUS=\"USIM\"\r\n";
  SendCmd(tmp);
      if(check(1,lte.rx_buf)==0)
      {
        

        S2L_LOG(lte.rx_buf);
        return 0;
      }
      return 1;
}
uint8_t CheckAT()
{
      uint8_t *tmp="AT\r\n";      
      SendCmd(tmp);
      if(check(0,lte.rx_buf)==0)
      {
        S2L_LOG(lte.rx_buf);
        return 0;
      }
      return 1;
}


uint8_t* SocketRead()
{
  uint8_t* rx;
  uint8_t* data;
  
  uint16_t len;
  uint16_t index;
  
  data=NULL;
  //memset(lte.rx_buf,0,BUF_SIZE);
  SendCmd((uint8_t*)ltecmd[9]);
  rx=lte.rx_buf;
  for(index=0;index<lte.length;index++)
  {
    if(rx[index]==':') 
      data=rx+index+1;
    if(rx[index]=='\r'&&rx[index+1]=='\n'&&rx[index+2]=='\r'&&rx[index+3]=='\n')
    {
      rx[index]='\0';
    }
  }
  
  return data;
}


uint8_t SocketWrite(uint8_t *data)
{
  uint8_t* all;
  uint32_t len;
  uint8_t* rx;
  uint8_t* cmd="AT^SISW=0,";
  len=strlen(data)+strlen(cmd)+3;//+('\0')+'\r'+'\n'
  all=pvPortMalloc(len);
  if(all==NULL)
  {
    while(1);
  }
  strcpy(all,cmd);
  strcat(all,data);
  all[len-1]='\0';
  all[len-2]='\n';
  all[len-3]='\r';
  //printf("%s\n",all);
  SendCmd(all);
  vPortFree(all);
  rx=lte.rx_buf;
  if(check(0,rx)==0)
  {
    S2L_LOG(rx);
  }
  else
    return 1;
  return 0;
}
static void Num2Str(uint8_t num,uint8_t* str)
{
  uint8_t tmp[3]={0};
  sprintf(tmp,"%x",num);
  if(tmp[1]=='\0')
  {
    tmp[1]=tmp[0];
    tmp[0]=0x30;
    tmp[2]='\0';
  }
  memcpy(str,tmp,2);
}

uint8_t SocketWriteBin(uint8_t *data,uint8_t data_len)
{
  //memset(lte.rx_buf,0,BUF_SIZE);
  uint8_t* all;
  uint32_t len;
  const char* cmd="AT^SISH=%d(0),";
  uint16_t cmd_len;
  cmd_len=strlen(cmd);
  len=data_len*2+cmd_len+3;//+('\0')+'\r'+'\n'
  all=pvPortMalloc(len);
  strcpy(all,cmd);
  for(int i=0;i<data_len;i++)
  {
    uint8_t tmp[2];
    Num2Str(data[i],tmp);
    all[cmd_len+i*2]=tmp[0];
    all[cmd_len+i*2+1]=tmp[1];
  }
  all[len-1]='\0';
  all[len-2]='\n';
  all[len-3]='\r';
  len=strlen((const char*)all);
  //memset(lte.rx_buf,0,BUF_SIZE);
  SendCmd(all);
  uint8_t write_cnt=0;
  do
  {
    
    if(check(0,lte.rx_buf)==0)
    {

      S2L_LOG("write_bin sucess!\r\n");
      vPortFree(all);
      return 0;
    }
    write_cnt++;
  }
  while(write_cnt<5);
  vPortFree(all);
  return 1;
}
uint8_t SocketOpen()
{

  uint8_t* rx;
  //memset(lte.rx_buf,0,BUF_SIZE);
  //HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
  //HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[8],strlen(ltecmd[8]),200);
  SendCmd((uint8_t*)ltecmd[8]);
  rx=lte.rx_buf;
  if(!check(0,rx))
  {

    S2L_LOG(rx);
  }
  else
  {
    S2L_LOG(rx);
    return 1;
  }
  return 0;
}
uint8_t SocketClose()
{
  uint8_t* rx;
  //memset(lte.rx_buf,0,BUF_SIZE);
//  HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
//  HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[3],strlen(ltecmd[3]),200);
//  osDelay(nop_time);
  SendCmd((uint8_t*)ltecmd[3]);
  rx=lte.rx_buf;
  if(!check(0,rx))
  {

    S2L_LOG(rx);
    return 0;
  }
  else
    return 1;
  
}

uint8_t SocketInit()
{
  uint8_t* rx;
  //tcp
  //memset(lte.rx_buf,0,BUF_SIZE);
//  HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
//  HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[4],len,200);
//  osDelay(nop_time);
//  rx=read();
  SendCmd((uint8_t*)ltecmd[4]);
  rx=lte.rx_buf;
  if(check(0,rx)==0)
  {

  S2L_LOG(rx);
  }
  else 
    return 1;
  //ip address
  //memset(lte.rx_buf,0,BUF_SIZE);
//  HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
//  HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[5],len,200);
//  osDelay(nop_time);
//  rx=read();
  SendCmd((uint8_t*)ltecmd[5]);
  rx=lte.rx_buf;
  if(check(0,rx)==0)
  {

    S2L_LOG(rx);
  }
  else
    return 1;
  //port 
  //memset(lte.rx_buf,0,BUF_SIZE);
//  HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
//  HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[6],len,200);
//  osDelay(nop_time);
//  rx=read();
  SendCmd((uint8_t*)ltecmd[6]);
  rx=lte.rx_buf;
  if(check(0,rx)==0)
  {

    S2L_LOG(rx);
  }
  else
    return 1;
  //connect init
  //memset(lte.rx_buf,0,BUF_SIZE);
//  HAL_UART_Receive_DMA(&LTE_UART,lte.rx_buf,BUF_SIZE);
//  HAL_UART_Transmit(&LTE_UART,(uint8_t*)ltecmd[7],strlen(ltecmd[7]),200);
//  osDelay(nop_time);
//  rx=read();
  SendCmd((uint8_t*)ltecmd[7]);
  rx=lte.rx_buf;
  if(check(0,rx)==0)
  {
    S2L_LOG(rx);

  }
  else
    return 1;
  return 0;
}
