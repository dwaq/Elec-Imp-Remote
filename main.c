/**
  ******************************************************************************
  * @file    main.c
  * @author  Dillon Nichols
  * @version V0.0
  * @date    07-20-2015
  * @brief   Project to control Elec-Imp-Relay from 32F429IDISCOVERY board
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

typedef enum
{
	MAIN = 0,
	MAIN_TOUCH = 1,
	TOGGLE = 2,
	TOGGLE_TOUCH = 3,
	BREW_NOW = 4,
	BREW_NOW_TOUCH = 5,
	BREW_DELAY = 6,
	BREW_DELAY_TOUCH = 7,
	BREW_CONFIRM = 8
} DISPLAY_STATES_TypeDef;

DISPLAY_STATES_TypeDef LCD_state = MAIN;

typedef enum
{
	DECREMENT = 0,
	INCREMENT = 1
} Unary_Operator_TypeDef;

Unary_Operator_TypeDef change = DECREMENT;

typedef enum
{
	HOURS = 0,
	MINUTES = 1
} Time_TypeDef;

Time_TypeDef time = HOURS;

uint8_t time_display[15] = "...hh..:..mm...";
uint8_t time_hours = 6;
uint8_t time_minutes = 50;
	
// for Delay()
static __IO uint32_t TimingDelay;
	
/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);
void modifyTime(Unary_Operator_TypeDef change, Time_TypeDef time);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  //uint16_t linenum = 0;
  static TP_STATE* TP_State; 
    
	// set systick to fire every 0.1ms
	SysTick_Config(18000);
	
  /* LCD initialization */
  LCD_Init();
  
  /* LCD Layer initialization */
  LCD_LayerInit();
    
  /* Enable the LTDC */
  LTDC_Cmd(ENABLE);
  
  /* Set LCD foreground layer */
  LCD_SetLayer(LCD_FOREGROUND_LAYER);
 
  /* Configure the IO Expander and display error if not OK */
  if (IOE_Config() != IOE_OK)
  {
    LCD_Clear(LCD_COLOR_RED);
    LCD_SetTextColor(LCD_COLOR_BLACK); 
    LCD_DisplayStringLine(LCD_LINE_6,(uint8_t*)"   IOE NOT OK      ");
    LCD_DisplayStringLine(LCD_LINE_7,(uint8_t*)"Reset the board   ");
    LCD_DisplayStringLine(LCD_LINE_8,(uint8_t*)"and try again     ");
		while(1);
  }
	
  while (1)
  {
    TP_State = IOE_TP_GetState();
		
		switch (LCD_state)
		{
			case MAIN:
				LCD_Clear(LCD_COLOR_WHITE);
			
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
			
				// current time at top
				LCD_DisplayStringLine(LINE(1), (uint8_t*)"........HH:MM..");
			
				// back button - for sizing
				//LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);

				LCD_DisplayStringLine(LINE(box1_line), (uint8_t*)".....TOGGLE....");
				LCD_DisplayStringLine(LINE(box2_line), (uint8_t*)"....BREW.NOW...");
				LCD_DisplayStringLine(LINE(box3_line), (uint8_t*)"...DELAY.BREW..");
				
				LCD_SetTextColor(LCD_COLOR_BLUE);
				// box 1
				LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
				
				// box 2
				LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
				
				// box 3
				LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
				
				LCD_SetTextColor(LCD_COLOR_RED);
				LCD_SetFont(&Font8x12);
				// temperature at the bottom
				LCD_DisplayStringLine(LINE(25), (uint8_t*)"...Current temperature: 72F...");
				
				LCD_state = MAIN_TOUCH;
			break;
			
			case MAIN_TOUCH:
				// box 1
				if ((TP_State->TouchDetected) && (TP_State->Y <= box1_y2) && (TP_State->Y >= box1_y1) && (TP_State->X >= box1_x1) && (TP_State->X <= box1_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					LCD_state = TOGGLE;
				}
				// box 2
				else if ((TP_State->TouchDetected) && (TP_State->Y <= box2_y2) && (TP_State->Y >= box2_y1) && (TP_State->X >= box2_x1) && (TP_State->X <= box2_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					LCD_state = BREW_NOW;
				}
				// box 3
				else if ((TP_State->TouchDetected) && (TP_State->Y <= box3_y2) && (TP_State->Y >= box3_y1) && (TP_State->X >= box3_x1) && (TP_State->X <= box3_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					LCD_state = BREW_DELAY;
				}
				break;
				
			case TOGGLE:
				LCD_Clear(LCD_COLOR_WHITE);
			
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
			
				// current time at top
				LCD_DisplayStringLine(LINE(1), (uint8_t*)"........HH:MM..");
			
				// back button
				LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);

				LCD_DisplayStringLine(LINE(box1_line), (uint8_t*)"....Turn.on....");
				LCD_DisplayStringLine(LINE(box2_line), (uint8_t*)"....Turn.off...");
				
				LCD_SetTextColor(LCD_COLOR_BLUE);
				// box 1
				LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
				
				// box 2
				LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
				
				LCD_SetTextColor(LCD_COLOR_RED);
				LCD_SetFont(&Font8x12);
				// temperature at the bottom
				LCD_DisplayStringLine(LINE(25), (uint8_t*)"...Current temperature: 72F...");
				
				LCD_state = TOGGLE_TOUCH;
				break;
				
			case TOGGLE_TOUCH:
				// box 1
				if ((TP_State->TouchDetected) && (TP_State->Y <= box1_y2) && (TP_State->Y >= box1_y1) && (TP_State->X >= box1_x1) && (TP_State->X <= box1_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					// add logic to actually toggle
					LCD_state = TOGGLE;
				}
				// box 2
				else if ((TP_State->TouchDetected) && (TP_State->Y <= box2_y2) && (TP_State->Y >= box2_y1) && (TP_State->X >= box2_x1) && (TP_State->X <= box2_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					// add logic to actually toggle
					LCD_state = TOGGLE;
				}
				// back button
				else if ((TP_State->TouchDetected) && (TP_State->Y <= back_y2) && (TP_State->Y >= back_y1) && (TP_State->X >= back_x3) && (TP_State->X <= back_x2))
				{
					// highlight button red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					LCD_state = MAIN;
				}
			break;

			case BREW_NOW:
				LCD_Clear(LCD_COLOR_WHITE);
			
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
			
				// current time at top
				LCD_DisplayStringLine(LINE(1), (uint8_t*)"........HH:MM..");
			
				// back button
				LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);

				LCD_DisplayStringLine(LINE(box1_line), (uint8_t*)".....2.cups....");
				LCD_DisplayStringLine(LINE(box2_line), (uint8_t*)".....3.cups....");
				LCD_DisplayStringLine(LINE(box3_line), (uint8_t*)".....4.cups....");
				
				LCD_SetTextColor(LCD_COLOR_BLUE);
				// box 1
				LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
				
				// box 2
				LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
				
				// box 3
				LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
				
				LCD_SetTextColor(LCD_COLOR_RED);
				LCD_SetFont(&Font8x12);
				// temperature at the bottom
				LCD_DisplayStringLine(LINE(25), (uint8_t*)"...Current temperature: 72F...");
				
				LCD_state = BREW_NOW_TOUCH;
			break;
			
			case BREW_NOW_TOUCH:
				// box 1
				if ((TP_State->TouchDetected) && (TP_State->Y <= box1_y2) && (TP_State->Y >= box1_y1) && (TP_State->X >= box1_x1) && (TP_State->X <= box1_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box1_x1, box1_y1, box1_y2-box1_y1, box1_x2-box1_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					// add logic to send data
					LCD_state = BREW_NOW;
				}
				// box 2
				else if ((TP_State->TouchDetected) && (TP_State->Y <= box2_y2) && (TP_State->Y >= box2_y1) && (TP_State->X >= box2_x1) && (TP_State->X <= box2_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box2_x1, box2_y1, box2_y2-box2_y1, box2_x2-box2_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					// add logic to send data
					LCD_state = BREW_NOW;
				}
				// box 3
				else if ((TP_State->TouchDetected) && (TP_State->Y <= box3_y2) && (TP_State->Y >= box3_y1) && (TP_State->X >= box3_x1) && (TP_State->X <= box3_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					// add logic to send data
					LCD_state = BREW_NOW;
				}
				// back button
				else if ((TP_State->TouchDetected) && (TP_State->Y <= back_y2) && (TP_State->Y >= back_y1) && (TP_State->X >= back_x3) && (TP_State->X <= back_x2))
				{
					// highlight button red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);
					
					// delay so user can see
					Delay(3000);
					
					// change to next state
					LCD_state = MAIN;
				}
			break;
			
			case BREW_DELAY:
				LCD_Clear(LCD_COLOR_WHITE);
			
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
			
				// current time at top
				LCD_DisplayStringLine(LINE(1), (uint8_t*)"........HH:MM..");
			
				// back button
				LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);
			
				// select cups
				LCD_DisplayStringLine(LINE(11), (uint8_t*)"..2....3....4..");
				
				// display set time at middle
				LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);
				
				// left up arrow
				LCD_FillTriangle(16*3, 16*5, 16*4, 26*5, 26*5, 26*4);
				// right up arrow
				LCD_FillTriangle(16*10, 16*12, 16*11, 26*5, 26*5, 26*4);
				// left down arrow
				LCD_FillTriangle(16*3, 16*5, 16*4, 26*7, 26*7, 26*8);
				// right down arrow
				LCD_FillTriangle(16*10, 16*12, 16*11, 26*7, 26*7, 26*8);
				
				LCD_SetTextColor(LCD_COLOR_RED);
				LCD_SetFont(&Font8x12);
				
				// selection text
				LCD_DisplayStringLine(LINE(19), (uint8_t*)".....Select number of cups:...");
				
				// temperature at the bottom
				LCD_DisplayStringLine(LINE(25), (uint8_t*)"...Current temperature: 72F...");
				
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLUE);
				
				// left up button
				LCD_DrawRect(x1_l_u, y1_l_u, y2_l_u-y1_l_u, x2_l_u-x1_l_u);
				// right up button
				LCD_DrawRect(x1_r_u, y1_r_u, y2_r_u-y1_r_u, x2_r_u-x1_r_u);
				// left down button
				LCD_DrawRect(x1_l_d, y1_l_d, y2_l_d-y1_l_d, x2_l_d-x1_l_d);
				// right down button
				LCD_DrawRect(x1_r_d, y1_r_d, y2_r_d-y1_r_d, x2_r_d-x1_r_d);
				
				// 2 cups button
				LCD_DrawRect(x1_2_c, y1_2_c, y2_2_c-y1_2_c, x2_2_c-x1_2_c);
				// 3 cups button
				LCD_DrawRect(x1_3_c, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c);
				// 4 cups button
				LCD_DrawRect(x1_4_c, y1_4_c, y2_4_c-y1_4_c, x2_4_c-x1_4_c);
				
				LCD_state = BREW_DELAY_TOUCH;
			break;
				
			case BREW_DELAY_TOUCH:
				// left up
				if ((TP_State->TouchDetected) && (TP_State->Y <= y2_l_u) && (TP_State->Y >= y1_l_u) && (TP_State->X >= x1_l_u) && (TP_State->X <= x2_l_u))
				{
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(x1_l_u, y1_l_u, y2_l_u-y1_l_u, x2_l_u-x1_l_u);

					modifyTime(INCREMENT, HOURS);

					LCD_SetTextColor(LCD_COLOR_BLACK);
					LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);

					Delay(3000);

					LCD_SetTextColor(LCD_COLOR_BLUE);
					LCD_DrawRect(x1_l_u, y1_l_u, y2_l_u-y1_l_u, x2_l_u-x1_l_u);
				}
				// left down
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_l_d) && (TP_State->Y >= y1_l_d) && (TP_State->X >= x1_l_d) && (TP_State->X <= x2_l_d))
				{
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(x1_l_d, y1_l_d, y2_l_d-y1_l_d, x2_l_d-x1_l_d);

					modifyTime(DECREMENT, HOURS);

					LCD_SetTextColor(LCD_COLOR_BLACK);
					LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);

					Delay(3000);

					LCD_SetTextColor(LCD_COLOR_BLUE);
					LCD_DrawRect(x1_l_d, y1_l_d, y2_l_d-y1_l_d, x2_l_d-x1_l_d);
				}
				// right up
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_r_u) && (TP_State->Y >= y1_r_u) && (TP_State->X >= x1_r_u) && (TP_State->X <= x2_r_u))
				{
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(x1_r_u, y1_r_u, y2_r_u-y1_r_u, x2_r_u-x1_r_u);

					modifyTime(INCREMENT, MINUTES);

					LCD_SetTextColor(LCD_COLOR_BLACK);
					LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);

					Delay(3000);

					LCD_SetTextColor(LCD_COLOR_BLUE);
					LCD_DrawRect(x1_r_u, y1_r_u, y2_r_u-y1_r_u, x2_r_u-x1_r_u);
				}
				// right down
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_r_d) && (TP_State->Y >= y1_r_d) && (TP_State->X >= x1_r_d) && (TP_State->X <= x2_r_d))
				{
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(x1_r_d, y1_r_d, y2_r_d-y1_r_d, x2_r_d-x1_r_d);

					modifyTime(DECREMENT, MINUTES);

					LCD_SetTextColor(LCD_COLOR_BLACK);
					LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);

					Delay(3000);

					LCD_SetTextColor(LCD_COLOR_BLUE);
					LCD_DrawRect(x1_r_d, y1_r_d, y2_r_d-y1_r_d, x2_r_d-x1_r_d);
				}
				//  2 cups
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_2_c) && (TP_State->Y >= y1_2_c) && (TP_State->X >= x1_2_c) && (TP_State->X <= x2_2_c))
				{
					LCD_state = BREW_CONFIRM;
				}
				// 3 cups
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_3_c) && (TP_State->Y >= y1_3_c) && (TP_State->X >= x1_3_c) && (TP_State->X <= x2_3_c))
				{
					LCD_state = BREW_CONFIRM;
				}
				// 4 cups
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_4_c) && (TP_State->Y >= y1_4_c) && (TP_State->X >= x1_4_c) && (TP_State->X <= x2_4_c))
				{
					LCD_state = BREW_CONFIRM;
				}
			break;
						
			case BREW_CONFIRM:
				LCD_Clear(LCD_COLOR_WHITE);

				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
				LCD_DisplayStringLine(LINE(5), (uint8_t*)".I will brew X.");
				LCD_DisplayStringLine(LINE(6), (uint8_t*)".cups at HH:MM.");

				LCD_DisplayStringLine(LINE(11), (uint8_t*)"....Thanks!....");

				LCD_SetTextColor(LCD_COLOR_BLUE);
				#define thanks_x_shift 16*2
				LCD_DrawRect(x1_3_c-thanks_x_shift, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c+thanks_x_shift*2);
	
				Delay(10000);
			LCD_state = MAIN;
			break;
		}

	}
}

void modifyTime(Unary_Operator_TypeDef change, Time_TypeDef time)
{
	switch (change)
	{
		case DECREMENT:
			switch (time)
			{
				case HOURS:
					// if decrementing hours and hours is 0, go back to 23 hours
					if(time_hours == 0)
					{
						time_hours = 23;
					}
					else
					{
						time_hours--;
					}
				break;
				
				case MINUTES:
					// if decrementing minutes and minutes is 0, go back to 59 minutes
					if (time_minutes == 0)
					{
						time_minutes = 55;
						
						// and decrement the hour too, ensuring it won't go negative either
						if (time_hours != 1)
						{
							time_hours--;
						}
						else
						{
							time_hours = 23;
						}
					}
					else
					{
						time_minutes-=5;
					}
				break;
			}
		break;
			
		case INCREMENT:
			switch (time)
			{
				case HOURS:
					time_hours++;
				break;
				
				case MINUTES:
					time_minutes+=5;
				break;
			}
			break;
	}
	
	// turn 60 minutes into an hour
	if (time_minutes == 60)
	{
		time_hours++;
		time_minutes = 0;
	}
	
	// reset clock after midnight
	if (time_hours == 24)
	{
		time_hours = 0;
	}
	
	// add 0x30 to decimal numbers for ascii code
	time_display[3] = (time_hours/10)+0x30;
	time_display[4] = (time_hours%10)+0x30;
	
	time_display[10] = (time_minutes/10)+0x30;
	time_display[11] = (time_minutes%10)+0x30;
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
	TimingDelay = nTime;

	while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/**********************************END OF FILE*********************************/
