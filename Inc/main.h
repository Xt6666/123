#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

void Error_Handler(void);

#define MAX_30102_INT_Pin GPIO_PIN_9
#define MAX_30102_INT_GPIO_Port GPIOB
#define LCD_PWR_Pin GPIO_PIN_15
#define LCD_PWR_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_6
#define LCD_RST_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_7
#define LCD_DC_GPIO_Port GPIOC

#ifdef __cplusplus
}
#endif

#endif 
