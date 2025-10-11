/*
 * main.c - part of f303k_nucleo/overlay
 */

#include <stdlib.h>
#include "stm32f3xx_hal.h"
#include "led.h"
#include "usart.h"
#include "printf.h"


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
#if 0
  /* HSE 8MHz -> 72MHz */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
#else
  /* HSI 64MHz */
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* HSI Oscillator already ON after system reset, activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
#endif
}

/*
 * overlays
 */
void __attribute__((section(".ovly0_code"))) __attribute__((used)) overlay0_func(void)
{
	printf("Hello from Overlay 0\n\r");
}

void __attribute__((section(".ovly1_code"))) __attribute__((used)) overlay1_func(void)
{
	printf("Hello from Overlay 1\n\r");
}

/* table of function ptrs to overlays */
void (*ovl_tbl[])(void) = {
	overlay0_func,
	overlay1_func,
};

extern uint32_t _ovly_table[2][3];

/*
 * load overlay to CCMRAM
 */
void load_overlay(int idx)
{
	printf("Loading Overlay %d from 0x%08X to 0x%08X of length 0x%08x\n\r", 
		idx, _ovly_table[idx][2], _ovly_table[idx][0], _ovly_table[idx][1]);

	memcpy(_ovly_table[idx][0], _ovly_table[idx][2], _ovly_table[idx][1]);
}

/*
 * main routine
 */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* init the UART for diagnostics */
	setup_usart();
	init_printf(0,usart_putc);
	printf("\n\n\rF303K8 Nucleo Overlay\n\r");
	printf("\n");
	printf("SYSCLK = %d\n\r", HAL_RCC_GetSysClockFreq());
	printf("\n");
	
	/* initialize LED */
	LEDInit();
	printf("LED initialized\n\r");
	
	for(int x=0;x<2;x++)
	{
		for(int y=0;y<3;y++)
		{
			printf("Overlay Table[%d][%d] = 0x%08X\n\r", x, y, _ovly_table[x][y]);
		}
	}
	
	load_overlay(0);
	(*ovl_tbl[0])();
	load_overlay(1);
	(*ovl_tbl[1])();

    /* Infinite loop */
	printf("Looping...\n\r");
    while (1)
    {
		LEDToggle();
		HAL_Delay(100);
    }
}

/*
 * needed by HAL
 */
void SysTick_Handler(void)
{
	HAL_IncTick();
}

