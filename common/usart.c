/*
 * usart.c - uart diag support for F373 TG
 * 12-13-2018 E. Brombaugh
 */

#include <stdio.h>
#include "usart.h"

/* uncomment this to enable RX */
#define USART_USE_RX

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA

#ifdef USART_USE_RX
#define USART_RX_Pin GPIO_PIN_15
#define USART_RX_GPIO_Port GPIOA

/* uncomment this to use IRQs for RX buffering */
#define USART_USE_IRQ

#ifdef USART_USE_IRQ
uint8_t RX_buffer[256];
uint8_t *RX_wptr, *RX_rptr;
#endif
#endif

UART_HandleTypeDef huart2;

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Hang forever */
  while(1)
  {
  }
}

/* USART setup */
void setup_usart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Setup USART GPIO */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.Pin = USART_TX_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(USART_TX_GPIO_Port, &GPIO_InitStructure);

#ifdef USART_USE_RX
	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.Pin = USART_RX_Pin;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(USART_RX_GPIO_Port, &GPIO_InitStructure);
#endif
	
	/* Setup USART */
	__HAL_RCC_USART2_CLK_ENABLE();

	/* USART = 115k-8-N-1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	
#ifdef USART_USE_IRQ
	/* init RX buffer write/read pointers*/
	RX_wptr = &RX_buffer[0];
	RX_rptr = &RX_buffer[0];

	/* Enable UART RXNE IRQ */
	SET_BIT(huart2.Instance->CR1, USART_CR1_RXNEIE);
	NVIC_SetPriority(USART2_IRQn, 0x03);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
#endif
}

/*
 * output for tiny printf
 */
void usart_putc(void* p, char c)
{
	while(__HAL_USART_GET_FLAG(&huart2, USART_FLAG_TC) == RESET)
	{
	}
	USART2->TDR = c;
}

/*
 * input
 */
int usart_getc(void)
{
#ifdef USART_USE_RX
#ifdef USART_USE_IRQ
	/* Interrupt version */
	int retval;
	
	/* check if there's data in the buffer */
	if(RX_rptr != RX_wptr)
	{
		/* get the data */
		retval = *RX_rptr++;
		
		/* wrap the pointer */
		if((RX_rptr - &RX_buffer[0])>=256)
			RX_rptr = &RX_buffer[0];
	}
	else
		retval = EOF;

	return retval;
#else
	/* Non-interrupt version */
	if(__HAL_USART_GET_FLAG(&huart2, USART_FLAG_RXNE) == SET)
		return USART2->RDR;
	else
		return EOF;
#endif
#else
    return 0;
#endif
}

#ifdef USART_USE_IRQ
/*
 * USART IRQ handler - used only for Rx for now
 */
void USART2_IRQHandler(void)
{
	/* RX available? */
	if(__HAL_USART_GET_FLAG(&huart2, USART_FLAG_RXNE) == SET)
	{
		/* get the character */
		uint8_t rxchar = USART2->RDR;

		/* check if there's room in the buffer */
		if((RX_wptr != RX_rptr-1) &&
           (RX_wptr - RX_rptr != 255))
		{
			/* Yes - Queue the new char */
			*RX_wptr++ = rxchar;
	
			/* Wrap pointer */
			if((RX_wptr - &RX_buffer[0])>=256)
				RX_wptr = &RX_buffer[0];
		}
	}

	/* TX Possible? */
	if(__HAL_USART_GET_FLAG(&huart2, USART_FLAG_TC) == SET)
	{
	}
}
#endif
