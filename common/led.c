/*
 * led.c - F303K8 Nucleo LED setup
 */

#include "led.h"

#define LD3_Pin GPIO_PIN_3
#define LD3_GPIO_Port GPIOB

/*
 * Initialize the breakout board LED
 */
void LEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO B Clock */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	/* Enable LD2 for output */
	GPIO_InitStructure.Pin =  LD3_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStructure.Pull = GPIO_NOPULL ;
	HAL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStructure);
}

/*
 * Turn on LED
 */
void LEDOn(void)
{
	LD3_GPIO_Port->BSRR = LD3_Pin;
}

/*
 * Turn off LED
 */
void LEDOff(void)
{
	LD3_GPIO_Port->BSRR = LD3_Pin;
}

/*
 * Toggle LED
 */
void LEDToggle(void)
{
	LD3_GPIO_Port->ODR ^= LD3_Pin;
}

