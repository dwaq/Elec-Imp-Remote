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
#define ABS(x)                     (x < 0) ? (-x) : x
#define L3G_Sensitivity_250dps     (float)114.285f        /*!< gyroscope sensitivity with 250 dps full scale [LSB/dps]  */
#define L3G_Sensitivity_500dps     (float)57.1429f        /*!< gyroscope sensitivity with 500 dps full scale [LSB/dps]  */
#define L3G_Sensitivity_2000dps    (float)14.285f         /*!< gyroscope sensitivity with 2000 dps full scale [LSB/dps] */
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
	BREW_DELAY_CONFIRM = 8,
	BREW_DELAY_CONFIRM_TOUCH = 9
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
uint8_t time_minutes = 45;

// temporary storage variable for reads from Gyro
uint8_t tmpbuffer[1] ={0};
// stores fahrenheit value after being converted from Gyro
uint8_t temperature_F = 0;
// to display accurate temperature at the bottom of the display
uint8_t temperature_display[30] = "   Current temperature: xxF   ";

float Buffer[6];
float Gyro[3];
float X_BiasError, Y_BiasError, Z_BiasError = 0.0;

// counting how long the board isn't moving
uint32_t still_counter = 0;

// for Delay()
static __IO uint32_t TimingDelay;
	
/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);
void modifyTime(Unary_Operator_TypeDef change, Time_TypeDef time);
void USART_print(USART_TypeDef* USARTx, volatile char *s);
static void Demo_GyroConfig(void);
static void GyroReadTemperature(void);
static void Demo_GyroReadAngRate (float* pfData);
static void Gyro_SimpleCalibration(float* GyroData);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	
  //uint16_t linenum = 0;
  static TP_STATE* TP_State; 
    
	// set systick to fire every 0.1ms
	SysTick_Config(18000);
	
	/* set up UART1 */
	
	//enable the GPIO clocks for PA9 and PA10
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	// Connect the UART pins to the peripherals' Alternate Function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	
	// set up the UART pins
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Enable UART peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// program UART settings
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	// Enable UART RX interrupt
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	
	// print something to show it's working
	USART_print(USART1, "Hello World!\r\n");

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
	
	/* Gyroscope configuration */
  Demo_GyroConfig();
	
	 /* Gyroscope calibration */
  Gyro_SimpleCalibration(Gyro);
	
	// use the LED as a designator for the backlight
	STM_EVAL_LEDInit(LED3);
	
	// put the time into the variable as a default
	modifyTime(INCREMENT, MINUTES);
	
  while (1)
  {
    TP_State = IOE_TP_GetState();
		
		Demo_GyroReadAngRate(Buffer);
		
		// not moving
		if ((ABS(Buffer[0]) < 2)&&(ABS(Buffer[1]) < 2)&&(ABS(Buffer[2]) < 1))
		{
			still_counter++;
			// after being still for X times, turn the light off
			if (still_counter == 10000)
			{
				STM_EVAL_LEDOff(LED3);
			}
		}
		else
		{
			still_counter = 0;
			STM_EVAL_LEDOn(LED3);
		}
		
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
				
				// display current temperature at the bottom
				GyroReadTemperature();
				
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
				
				// display current temperature at the bottom
				GyroReadTemperature();
				
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
				
				// display current temperature at the bottom
				GyroReadTemperature();
				
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
				
				// display current temperature at the bottom
				GyroReadTemperature();
				
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
					LCD_state = BREW_DELAY_CONFIRM;
				}
				// 3 cups
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_3_c) && (TP_State->Y >= y1_3_c) && (TP_State->X >= x1_3_c) && (TP_State->X <= x2_3_c))
				{
					LCD_state = BREW_DELAY_CONFIRM;
				}
				// 4 cups
				else if ((TP_State->TouchDetected) && (TP_State->Y <= y2_4_c) && (TP_State->Y >= y1_4_c) && (TP_State->X >= x1_4_c) && (TP_State->X <= x2_4_c))
				{
					LCD_state = BREW_DELAY_CONFIRM;
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
						
			case BREW_DELAY_CONFIRM:
				LCD_Clear(LCD_COLOR_WHITE);
			
				LCD_SetFont(&Font16x24);
				LCD_SetTextColor(LCD_COLOR_BLACK);
			
				// current time at top
				LCD_DisplayStringLine(LINE(1), (uint8_t*)"........HH:MM..");
			
				// back button
				LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);

				LCD_DisplayStringLine(LINE(5), (uint8_t*)".I will brew X.");
				LCD_DisplayStringLine(LINE(6), (uint8_t*)".cups at HH:MM.");
			
				LCD_DisplayStringLine(LINE(box3_line), (uint8_t*)"....CONFIRM....");
				
				LCD_SetTextColor(LCD_COLOR_BLUE);
				
				// box 3
				LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
				
				// display current temperature at the bottom
				GyroReadTemperature();
				
				Delay(10000);
				
				LCD_state = BREW_DELAY_CONFIRM_TOUCH;
			break;
			
			case BREW_DELAY_CONFIRM_TOUCH:
				// box 3
				if ((TP_State->TouchDetected) && (TP_State->Y <= box3_y2) && (TP_State->Y >= box3_y1) && (TP_State->X >= box3_x1) && (TP_State->X <= box3_x2))
				{
					// highlight box red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawRect(box3_x1, box3_y1, box3_y2-box3_y1, box3_x2-box3_x1);
					
					// delay so user can see
					Delay(3000);
					
					// go back to main
					LCD_state = MAIN;
				}
				// back button
				else if ((TP_State->TouchDetected) && (TP_State->Y <= back_y2) && (TP_State->Y >= back_y1) && (TP_State->X >= back_x3) && (TP_State->X <= back_x2))
				{
					// highlight button red
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_FillTriangle(back_x1, back_x2, back_x3, back_y1, back_y2, back_y3);
					
					// delay so user can see
					Delay(3000);
					
					// go back a screen
					LCD_state = BREW_DELAY;
				}
			break;
		}

	}
}

/**
* @brief  Change the delay time by a certain amount
* @param  Unary_Operator_TypeDef : choose to increase or decrease the time
* @param  Time_TypeDef : choose to change hour or minute
* @retval None
*/
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
* @brief  Transmit a string of characters via the USART
* @param  USARTx : can be any of the USARTs e.g. USART1, USART2 etc
* @param  char *s : pointer to the string you want to send
* @retval None
*/
void USART_print(USART_TypeDef* USARTx, volatile char *s)
{
	while(*s)
	{
		// wait until transmission is complete
		while( !(USARTx->SR & USART_FLAG_TC) );
		// send next data
		USART_SendData(USARTx, *s);
		// go to next char
		*s++;
	}
}

/**
* @brief  Configure the Mems to gyroscope application.
* @param  None
* @retval None
*/
static void Demo_GyroConfig(void)
{
  L3GD20_InitTypeDef L3GD20_InitStructure;
  L3GD20_FilterConfigTypeDef L3GD20_FilterStructure;

  /* Configure Mems L3GD20 */
  L3GD20_InitStructure.Power_Mode = L3GD20_MODE_ACTIVE;
  L3GD20_InitStructure.Output_DataRate = L3GD20_OUTPUT_DATARATE_1;
  L3GD20_InitStructure.Axes_Enable = L3GD20_AXES_ENABLE;
  L3GD20_InitStructure.Band_Width = L3GD20_BANDWIDTH_4;
  L3GD20_InitStructure.BlockData_Update = L3GD20_BlockDataUpdate_Continous;
  L3GD20_InitStructure.Endianness = L3GD20_BLE_LSB;
  L3GD20_InitStructure.Full_Scale = L3GD20_FULLSCALE_500; 
  L3GD20_Init(&L3GD20_InitStructure);
  
  L3GD20_FilterStructure.HighPassFilter_Mode_Selection =L3GD20_HPM_NORMAL_MODE_RES;
  L3GD20_FilterStructure.HighPassFilter_CutOff_Frequency = L3GD20_HPFCF_0;
  L3GD20_FilterConfig(&L3GD20_FilterStructure) ;
  
  L3GD20_FilterCmd(L3GD20_HIGHPASSFILTER_ENABLE);
}

/**
* @brief  Calculate the angular Data rate Gyroscope.
* @param  pfData : Data out pointer
* @retval None
*/
static void Demo_GyroReadAngRate (float* pfData)
{
  uint8_t tmpbuffer[6] ={0};
  int16_t RawData[3] = {0};
  uint8_t tmpreg = 0;
  float sensitivity = 0;
  int i =0;
  
  L3GD20_Read(&tmpreg,L3GD20_CTRL_REG4_ADDR,1);
  
  L3GD20_Read(tmpbuffer,L3GD20_OUT_X_L_ADDR,6);
  
  /* check in the control register 4 the data alignment (Big Endian or Little Endian)*/
  if(!(tmpreg & 0x40))
  {
    for(i=0; i<3; i++)
    {
      RawData[i]=(int16_t)(((uint16_t)tmpbuffer[2*i+1] << 8) + tmpbuffer[2*i]);
    }
  }
  else
  {
    for(i=0; i<3; i++)
    {
      RawData[i]=(int16_t)(((uint16_t)tmpbuffer[2*i] << 8) + tmpbuffer[2*i+1]);
    }
  }
  
  /* Switch the sensitivity value set in the CRTL4 */
  switch(tmpreg & 0x30)
  {
  case 0x00:
    sensitivity=L3G_Sensitivity_250dps;
    break;
    
  case 0x10:
    sensitivity=L3G_Sensitivity_500dps;
    break;
    
  case 0x20:
    sensitivity=L3G_Sensitivity_2000dps;
    break;
  }
  /* divide by sensitivity */
  for(i=0; i<3; i++)
  {
  pfData[i]=(float)RawData[i]/sensitivity;
  }
	
	Buffer[0] = (int8_t)Buffer[0] - (int8_t)Gyro[0];
  Buffer[1] = (int8_t)Buffer[1] - (int8_t)Gyro[1];
}

/**
* @brief  Calculate offset of the angular Data rate Gyroscope.
* @param  GyroData : Data out pointer
* @retval None
*/
static void Gyro_SimpleCalibration(float* GyroData)
{
  uint32_t BiasErrorSplNbr = 500;
  int i = 0;
  
  for (i = 0; i < BiasErrorSplNbr; i++)
  {
    Demo_GyroReadAngRate(GyroData);
    X_BiasError += GyroData[0];
    Y_BiasError += GyroData[1];
    Z_BiasError += GyroData[2];
  }
  /* Set bias errors */
  X_BiasError /= BiasErrorSplNbr;
  Y_BiasError /= BiasErrorSplNbr;
  Z_BiasError /= BiasErrorSplNbr;
  
  /* Get offset value on X, Y and Z */
  GyroData[0] = X_BiasError;
  GyroData[1] = Y_BiasError;
  GyroData[2] = Z_BiasError;
}

/**
* @brief  Read and calculate the temperature from the Gyroscope.
* @param  None
* @retval None
*/
static void GyroReadTemperature(void)
{
	// temporary value for calculations
	int8_t temp = 0;
	
	// read 1 byte from temperature register
	L3GD20_Read(tmpbuffer,L3GD20_OUT_TEMP_ADDR,1);

	// http://forums.parallax.com/discussion/comment/1165144/#Comment_1165144
	temp = 25 - tmpbuffer[0];
	#define correction_factor 0		// example used -4: need to calibrate mine
	temp = (temp + 25) + correction_factor; //Deg C
	temperature_F = (temp * (9/5)) + 32; //Deg F
	
	// write temperature across bottom of screen
	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_SetFont(&Font8x12);
	temperature_display[24] = (temperature_F/10)+0x30;
	temperature_display[25] = (temperature_F%10)+0x30;
	LCD_DisplayStringLine(LINE(25), (uint8_t*)temperature_display);
	
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
