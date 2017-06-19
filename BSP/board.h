#ifndef BOARD_H
#define BOARD_H

#include "stm32l0xx_hal.h"


//LED




#define LED1_PORT GPIOB
#define LED1 GPIO_PIN_7



//IR Control
#define G_IR_EN_PORT GPIOA
#define G_IR_EN GPIO_PIN_0

#define P_IR_EN_PORT GPIOB
#define P_IR_EN GPIO_PIN_3


//4G Port
#define LTE_CLOSE_PORT GPIOA
#define LTE_CLOSE GPIO_PIN_8

#define LTE_RESET_PORT GPIOB
#define LTE_RESET GPIO_PIN_13

#define LTE_WAKEUP_PORT GPIOB
#define LTE_WAKEUP_PIN GPIO_PIN_15
//**************

//GPS Enable
#define GPS_EN_PORT GPIOB
#define GPS_EN GPIO_PIN_4

//MV Enable
#define MV_EN_PORT GPIOB
#define MV_EN GPIO_PIN_8

//S2L On_off
#define ON_OFF_PORT GPIOB
#define ON_OFF GPIO_PIN_12

//SPI_CS
#define SPI_CS_PORT GPIOA
#define SPI_CS GPIO_PIN_4

//LOGO_EN
#define LOGO_EN_PORT GPIOA
#define LOGO_EN GPIO_PIN_2
//UART_SUSPEND
#define USB_SUSPEND_PORT GPIOA
#define USB_SUSPEND GPIO_PIN_15
//CP_GPIO0
#define CP_GPIO0_PORT GPIOA
#define CP_GPIO0 GPIO_PIN_11
//CP GPIO1
#define CP_GPIO1_PORT GPIOA
#define CP_GPIO1 GPIO_PIN_12





/***************************input exti*****************************/
//ac interrupt 1
#define AC_IT1 GPIOB
#define AC_IT1_PIN GPIO_PIN_0
//ac interrupt 1
#define AC_IT2 GPIOB
#define AC_IT2_PIN GPIO_PIN_2
//lte
#define LTE_IT GPIOB
#define LTE_IT_PIN GPIO_PIN_14
//Microwave
#define MW_IT GPIOB
#define MW_IT_PIN GPIO_PIN_9
//SIM CARD
#define SIM_IT GPIOC
#define SIM_IT_PIN GPIO_PIN_13












#endif