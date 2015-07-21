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
#include "stm32f429i_discovery_l3gd20.h"

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

// back button triangle location
#define back_x1 (font_w*3)
#define back_x2 (font_w*3)
#define back_x3 (font_w*0.5)
#define back_y1 (font_h*0.5)
#define back_y2 (font_h*2)
#define back_y3 (font_h*1.25)

// BREW_DELAY screen has many different different objects
// left up button
#define x1_l_u 16*2
#define x2_l_u 16*6
#define y1_l_u 26*3.5
#define y2_l_u 26*5.5

// right up button
#define xshift_r_u 16*7
#define x1_r_u (x1_l_u+xshift_r_u)
#define x2_r_u (x2_l_u+xshift_r_u)
#define y1_r_u y1_l_u
#define y2_r_u y2_l_u

// left down button
#define x1_l_d 16*2
#define x2_l_d 16*6
#define y1_l_d 26*6.5
#define y2_l_d 26*8.5

// right down button
#define xshift_r_d 16*7
#define x1_r_d (x1_l_d+xshift_r_d)
#define x2_r_d (x2_l_d+xshift_r_d)
#define y1_r_d y1_l_d
#define y2_r_d y2_l_d

// 2 cups button
#define x1_2_c 16*0.5
#define x2_2_c 16*4.5
#define y1_2_c 26*9.5
#define y2_2_c 26*11.5

// 3 cups button
#define x1_3_c 16*5.5
#define x2_3_c 16*9.5
#define y1_3_c 26*9.5
#define y2_3_c 26*11.5

// 4 cups button
#define x1_4_c 16*10.5
#define x2_4_c 16*14.5
#define y1_4_c 26*9.5
#define y2_4_c 26*11.5

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/ 
/* Exported functions ------------------------------------------------------- */
void TimingDelay_Decrement(void);

#endif /* __MAIN_H */

/**********************************END OF FILE*********************************/
