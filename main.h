/**
  ******************************************************************************
  * @file    main.h
  * @author  Dillon Nichols
  * @version V0.0
  * @date    07-20-2015
  * @brief   Project to control Elec-Imp-Relay from 32F429IDISCOVERY board
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdio.h>
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ioe.h"

/* Private define ------------------------------------------------------------*/

// used to align targets to normal font size
#define font_w 16
#define font_h 25

// display width  = 240px = x-direction
// display height = 320px = y-direction
#define max_width 240
#define max_height 320

// display normally consists of 3 touch targets
#define box1_line 4
#define box1_x1 font_w
#define box1_x2 (max_width-font_w)
#define box1_y1 (font_h*(box1_line-1))
#define box1_y2 (font_h*(box1_line+1))

#define box2_line 7
#define box2_x1 font_w
#define box2_x2 (max_width-font_w)
#define box2_y1 (font_h*(box2_line-1))
#define box2_y2 (font_h*(box2_line+1))

#define box3_line 10
#define box3_x1 font_w
#define box3_x2 (max_width-font_w)
#define box3_y1 (font_h*(box3_line-1))
#define box3_y2 (font_h*(box3_line+1))


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/ 
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);

#endif /* __MAIN_H */

/**********************************END OF FILE*********************************/