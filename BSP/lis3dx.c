#include "lis3dx.h"
#include "board.h"


#define STATUS_AUB 0X07
#define OUT_ADC1_L 0X08
#define OUT_ADC1_H 0X09
#define OUT_ADC2_L 0X0A
#define OUT_ADC2_H 0X0B
#define OUT_ADC3_L 0X0C
#define OUT_ADC3_H 0X0D

#define INT_COUNT 0X0E
#define WHO_AM_I  0X0F
#define TEMP_CFG  0X1F
#define CTRL_REG1 0X20
#define CTRL_REG2 0X21
#define CTRL_REG3 0X22
#define CTRL_REG4 0X23
#define CTRL_REG5 0X24
#define CTRL_REG6 0X25
#define REF_REG   0X26
#define STATUS_REG 0X27
#define OUT_X_L 0X28
#define OUT_X_H 0X29
#define OUT_Y_L 0X2A
#define OUT_Y_H 0X2B
#define OUT_Z_L 0X2C
#define OUT_Z_H 0X2D
#define FIFO_CTRL_RERG 0X2E
#define FIFO_SRC_REG 0X2F
#define INI1_CFG 0X30
#define INT1_SRC 0X31
#define INT1_THS 0X32
#define INT1_DURATION 0X33

#define CLICK_CFG 0X38
#define CLICK_SRC 0X39
#define CLICK_THS 0X3A
#define TIME_LIMIT 0X3B
#define TIME_LATENCY 0X3C
#define TIME_WINDOW 0X3D

//mode 
//1:increase
//0:remain addr
static void spi_read(uint8_t addr,uint8_t* data,uint8_t length,uint8_t mode);
static void spi_write(uint8_t addr,uint8_t* data,uint8_t length,uint8_t mode);
static uint8_t who_am_i();


extern SPI_HandleTypeDef hspi1;

Lis3dxTypeDef Lis3dx;

uint8_t Lis3dxInit()
{
  uint8_t err;
  err=who_am_i();
  if(err)
    return 1;
  Lis3dx.odr=ODR_50HZ;
  Lis3dx.scale=SCALE_2G;
  Lis3dx.upload=0;
  Lis3dx.ths=0x05;
  Lis3dhConfig(ODR_400HZ,SCALE_2G);
  Lis3dhThs(0x07);
  
  
  
  
  return 0;
  
}
int16_t accel[3];
void Lis3dhConfig(uint8_t odr,uint8_t scale)
{
  uint8_t ctrl_reg[5]=
  {
    odr|NORMAL_MODE|ZAXI_ENABLE|YAXI_ENABLE|XAXI_ENABLE,
    0x09,
    INT_AOI1,
    scale|HR_ENABLE,
    0x00
  };
//  uint8_t ctrl_reg[5]=
//  {
//    0x57,
//    0x00,
//    0x40,
//    0x00,
//    0x00,
//  };
  spi_write(CTRL_REG1,ctrl_reg,5,1);
  spi_read(REF_REG,ctrl_reg,1,1);//reset the hp
  
 
}
void Lis3dhThs(uint8_t level)
{
  uint8_t int1_cfg=0x2a;// x yz 
  spi_write(INI1_CFG,&int1_cfg,1,0);
  uint8_t int1_ths[2];
  int1_ths[0]=level;
  int1_ths[1]=0;
  spi_write(INT1_THS,int1_ths,2,1);
}
uint8_t ReadIntCnt()
{
  uint8_t cnt;
  spi_read(INT_COUNT,&cnt,1,0);
  return cnt;
}
void accel_read(int16_t* accel)
{
  uint8_t buffer[6];
  spi_read(OUT_X_L,buffer,6,1);
  accel[0]=(buffer[1]<<8)|buffer[0];
  accel[1]=(buffer[3]<<8)|buffer[2];
  accel[2]=(buffer[5]<<8)|buffer[4];
}

void accel_raw_read(uint8_t* raw)
{
  spi_read(OUT_X_L,raw,6,1);
}

static uint8_t who_am_i()
{
  uint8_t who;
  spi_read(WHO_AM_I,&who,1,0);
  if(who==0x33)
    return 0;
  else 
    return 1;
}

static void spi_read(uint8_t addr,uint8_t* data,uint8_t length,uint8_t mode)
{
  HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS,GPIO_PIN_RESET);
  if(mode)
    addr=0xc0|addr;
  else
    addr=0x80|addr;
  //Write the addr
  HAL_SPI_Transmit(&hspi1,&addr,1,100);
  //read
  HAL_SPI_Receive(&hspi1,data,length,200);
  HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS,GPIO_PIN_SET);
}



static void spi_write(uint8_t addr,uint8_t* data,uint8_t length,uint8_t mode)
{
  HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS,GPIO_PIN_RESET);
  if(mode)
    addr=0x40|addr;
  else
    addr=0x00|addr;
  //Write the addr
  HAL_SPI_Transmit(&hspi1,&addr,1,100);
  //read
  HAL_SPI_Transmit(&hspi1,data,length,200);
  HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS,GPIO_PIN_SET);
}