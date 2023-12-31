/**
 ******************************************************************************
 * @file    UART/UART_Printf/Src/main.c
 * @author  MCD Application Team
 * @brief   This example shows how to retarget the C library printf function
 *          to the UART.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "main.h"
#include "API_debounce.h"
#include "API_delay.h"
#include "API_lcd1602_i2c.h"
#include "API_uart.h"

/**
 * Define the debounce time for the FSM, the button state will be checked at
 * this time interval.
 */
#define DEBOUNCE_TIME_MS 40

// Forward declarations
static void SystemClock_Config(void);
static void Error_Handler(void);

/**
 * These functions prints to the UART when the button is pressed/released. For
 * this project is used as debug method.
 */
static void ButtonPressedCallback(void);
static void ButtonReleasedCallback(void);

int main(void) {

	HAL_Init(); // Initialize HAL

	SystemClock_Config();

	/**
	 * We will blink the BLUE led to detect if the program hangs. This is useful
	 * for debugging.
	 */
	BSP_LED_Init(LED2);

	// Init uart, if fail go to Error Handler.
	if (!uartInit()) {
		Error_Handler();
	}

	/**
	 * Initialize the I2C interface and the LCD1602 display.
	 */
	LCD1602_Init();

	// Configure button pin (GPIOA Pin 0) as input.
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

	// LEDs delays
	delay_t FSM_delay;
	delay_t button_pressed_delay;

	// Init delays
	delayInit(&FSM_delay, 0);
	delayInit(&button_pressed_delay, 0);

	debounceFSM_init();

	uint32_t current_frequency = 100;

	/**
	 * Set pressed and released callback. These functions will be called when
	 * those events happen.
	 */
	setPressedCallback(ButtonPressedCallback);
	setReleasedCallback(ButtonReleasedCallback);

	// Initialize on slide algorithm
	LCD1602_FSM_NextAlgorithm();

	/**
	 * We are reading from UART with interrups. So we need this line to be able to
	 * catch interrupts.
	 */
	__enable_irq();

	/* Infinite loop */
	while (1) {
		if (delayRead(&FSM_delay)) {
			delayWrite(&FSM_delay, DEBOUNCE_TIME_MS);
			debounceFSM_update();
		}

		if (delayRead(&button_pressed_delay)) {
			// Togle frequency if key pressed.
			if (readKey()) {
				if (current_frequency == 100) {
					current_frequency = 500;
				} else {
					current_frequency = 100;
				}
				LCD1602_FSM_NextAlgorithm();
			}

			BSP_LED_Toggle(LED2);
			delayWrite(&button_pressed_delay, current_frequency);
		}

		const char *uart_readed_string = readString();
		if (uart_readed_string != NULL) {
			LCD1602_AddToBuffer(uart_readed_string);
		}

		LCD1602_FSM_UpdateDisplay();
	}
}

/**
 * The callback should print to the UART the event of pressed and released.
 */
static void ButtonPressedCallback(void) {
	uartSendString((uint8_t*) "Button pressed\r\n");
}

static void ButtonReleasedCallback(void) {
	uartSendString((uint8_t*) "Button released\r\n");
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 180000000
 *            HCLK(Hz)                       = 180000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 360
 *            PLL_P                          = 2
 *            PLL_Q                          = 7
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device
	 is clocked below the maximum system frequency, to update the voltage scaling
	 value regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
	RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		/* Initialization Error */
		Error_Handler();
	}
}
/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
static void Error_Handler(void) {
	/* Turn LED2 on */
	BSP_LED_On(LED2);
	while (1) {
	}
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
