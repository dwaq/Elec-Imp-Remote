/**
  ******************************************************************************
  * @file    main.c
  * @author  Dillon Nichols
  * @version V1.0.1
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
static void TP_Config(void);
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
  
  /* Touch Panel configuration */
  TP_Config();
   
	/* Clear the LCD */ 
  LCD_Clear(LCD_COLOR_WHITE);
	
	LCD_SetFont(&Font16x24);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_DisplayStringLine(LINE(11), (uint8_t*)"..2....3....4..");
	
	// current time at top
	LCD_DisplayStringLine(LINE(2), (uint8_t*)"........HH:MM..");

	// display set time at middle
	LCD_DisplayStringLine(LINE(6), (uint8_t*)time_display);
	
	//void LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
	LCD_FillTriangle(16*3, 16*5, 16*4, 26*5, 26*5, 26*4);	// left up
	LCD_FillTriangle(16*10, 16*12, 16*11, 26*5, 26*5, 26*4);	// right up
	
	LCD_FillTriangle(16*3, 16*5, 16*4, 26*7, 26*7, 26*8);	// left down
	LCD_FillTriangle(16*10, 16*12, 16*11, 26*7, 26*7, 26*8);	// right down
	
	LCD_SetTextColor(LCD_COLOR_RED);
	// back button
	LCD_FillTriangle(16*3, 16*3, 16*0.5,  26*1.5, 26*3, 26*2.25);
	LCD_SetFont(&Font8x12);
	// some small text
	LCD_DisplayStringLine(LINE(1), (uint8_t*)"...Current temperature: 72F...");
	LCD_DisplayStringLine(LINE(19), (uint8_t*)"....Select number of cups:...");
	
	// X, Y, Height, Width
	LCD_SetFont(&Font16x24);
	LCD_SetTextColor(LCD_COLOR_BLUE);
	
	// left up
	#define x1_l_u 16*2
	#define x2_l_u 16*6
	#define y1_l_u 26*3.5
	#define y2_l_u 26*5.5
	LCD_DrawRect(x1_l_u, y1_l_u, y2_l_u-y1_l_u, x2_l_u-x1_l_u);
	
	// right up
	#define xshift_r_u 16*7
	#define x1_r_u (x1_l_u+xshift_r_u)
	#define x2_r_u (x2_l_u+xshift_r_u)
	#define y1_r_u y1_l_u
	#define y2_r_u y2_l_u
	LCD_DrawRect(x1_r_u, y1_r_u, y2_r_u-y1_r_u, x2_r_u-x1_r_u);
	
	// left down
	#define x1_l_d 16*2
	#define x2_l_d 16*6
	#define y1_l_d 26*6.5
	#define y2_l_d 26*8.5
	LCD_DrawRect(x1_l_d, y1_l_d, y2_l_d-y1_l_d, x2_l_d-x1_l_d);
	
	// right down
	#define xshift_r_d 16*7
	#define x1_r_d (x1_l_d+xshift_r_d)
	#define x2_r_d (x2_l_d+xshift_r_d)
	#define y1_r_d y1_l_d
	#define y2_r_d y2_l_d
	LCD_DrawRect(x1_r_d, y1_r_d, y2_r_d-y1_r_d, x2_r_d-x1_r_d);
	
	// 2 cups
	#define x1_2_c 16*0.5
	#define x2_2_c 16*4.5
	#define y1_2_c 26*9.5
	#define y2_2_c 26*11.5
	LCD_DrawRect(x1_2_c, y1_2_c, y2_2_c-y1_2_c, x2_2_c-x1_2_c);
	
	// 3 cups
	#define x1_3_c 16*5.5
	#define x2_3_c 16*9.5
	#define y1_3_c 26*9.5
	#define y2_3_c 26*11.5
	LCD_DrawRect(x1_3_c, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c);
	
	// 4 cups
	#define x1_4_c 16*10.5
	#define x2_4_c 16*14.5
	#define y1_4_c 26*9.5
	#define y2_4_c 26*11.5
	LCD_DrawRect(x1_4_c, y1_4_c, y2_4_c-y1_4_c, x2_4_c-x1_4_c);
	
	// example for drawing lines
	// X, Y, length, direction
	//LCD_DrawLine(40, 70, 220, LCD_DIR_VERTICAL);
  //LCD_DrawLine(0, 319, 240, LCD_DIR_HORIZONTAL);
		
  while (1)
  {
 
    TP_State = IOE_TP_GetState();
		
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
			/* Clear the LCD */ 
			LCD_Clear(LCD_COLOR_WHITE);
			
			LCD_SetFont(&Font16x24);
			LCD_SetTextColor(LCD_COLOR_BLACK);
			LCD_DisplayStringLine(LINE(5), (uint8_t*)".I will brew 2.");
			LCD_DisplayStringLine(LINE(6), (uint8_t*)".cups at HH:MM.");
						
			LCD_DisplayStringLine(LINE(11), (uint8_t*)"....Thanks!....");
			
			LCD_SetTextColor(LCD_COLOR_BLUE);
			#define thanks_x_shift 16*2
			LCD_DrawRect(x1_3_c-thanks_x_shift, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c+thanks_x_shift*2);
    }
		// 3 cups
		else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_3_c) && (TP_State->Y >= y1_3_c) && (TP_State->X >= x1_3_c) && (TP_State->X <= x2_3_c))
    {
			/* Clear the LCD */ 
			LCD_Clear(LCD_COLOR_WHITE);
			
			LCD_SetFont(&Font16x24);
			LCD_SetTextColor(LCD_COLOR_BLACK);
			LCD_DisplayStringLine(LINE(5), (uint8_t*)".I will brew 3.");
			LCD_DisplayStringLine(LINE(6), (uint8_t*)".cups at HH:MM.");
						
			LCD_DisplayStringLine(LINE(11), (uint8_t*)"....Thanks!....");
			
			LCD_SetTextColor(LCD_COLOR_BLUE);
			#define thanks_x_shift 16*2
			LCD_DrawRect(x1_3_c-thanks_x_shift, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c+thanks_x_shift*2);
    }
		// 4 cups
		else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_4_c) && (TP_State->Y >= y1_4_c) && (TP_State->X >= x1_4_c) && (TP_State->X <= x2_4_c))
    {
			/* Clear the LCD */ 
			LCD_Clear(LCD_COLOR_WHITE);
			
			LCD_SetFont(&Font16x24);
			LCD_SetTextColor(LCD_COLOR_BLACK);
			LCD_DisplayStringLine(LINE(5), (uint8_t*)".I will brew 4.");
			LCD_DisplayStringLine(LINE(6), (uint8_t*)".cups at HH:MM.");
			
			LCD_DisplayStringLine(LINE(11), (uint8_t*)"....Thanks!....");
			
			LCD_SetTextColor(LCD_COLOR_BLUE);
			#define thanks_x_shift 16*2
			LCD_DrawRect(x1_3_c-thanks_x_shift, y1_3_c, y2_3_c-y1_3_c, x2_3_c-x1_3_c+thanks_x_shift*2);
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
* @brief  Configure the IO Expander and the Touch Panel.
* @param  None
* @retval None
*/
static void TP_Config(void)
{
  /* Clear the LCD */ 
  LCD_Clear(LCD_COLOR_WHITE);
  
  /* Configure the IO Expander */
  if (IOE_Config() == IOE_OK)
  {   
		// set display defaults
  }  
  else
  {
    LCD_Clear(LCD_COLOR_RED);
    LCD_SetTextColor(LCD_COLOR_BLACK); 
    LCD_DisplayStringLine(LCD_LINE_6,(uint8_t*)"   IOE NOT OK      ");
    LCD_DisplayStringLine(LCD_LINE_7,(uint8_t*)"Reset the board   ");
    LCD_DisplayStringLine(LCD_LINE_8,(uint8_t*)"and try again     ");
  }
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
