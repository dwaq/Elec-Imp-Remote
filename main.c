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
	BREW_DELAY_TOUCH = 7
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
						time_minutes = 59;
						
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
						time_minutes--;
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
					time_minutes++;
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
