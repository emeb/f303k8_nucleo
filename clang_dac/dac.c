/*
 * dac.c - dac setup for F373 Mod2
 * 01-08-2019 E. Brombaugh
 */
 
#include "dac.h"
#include "audio.h"
#include "printf.h"
#include "cyclesleep.h"

/* DMA buffer for DAC & copy */
uint32_t dac1_buffer[DAC_BUFSZ], dac2_buffer[DAC_BUFSZ];
static TIM_HandleTypeDef TimHandle;

/*
 * init all three DAC channels
 */
void dac_init(void)
{
    int i;
	GPIO_InitTypeDef GPIO_InitStructure;
    DAC_HandleTypeDef DacHandle;
	DAC_ChannelConfTypeDef sConfig = {0};
	DMA_HandleTypeDef DmaHandle;
	TIM_MasterConfigTypeDef sMasterConfig;
    
    /* load something into the buffers */
    for(i=0;i<DAC_BUFSZ;i++)
    {
        uint32_t temp = (65536/DAC_BUFSZ)*i;
        
        dac1_buffer[i] = (temp<<16) + temp;
        dac2_buffer[i] = temp;
    }
	
	/* enable clocks for DAC1, DAC2, GPIOA ----------------------------*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_DAC1_CLK_ENABLE();
	__HAL_RCC_DAC2_CLK_ENABLE();
	
	/* Configure diagnostic output pin on PA11 ------------------------*/
	GPIO_InitStructure.Pin =  GPIO_PIN_11;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

#if 0
    /* test diag output */
    while(1)
    {
        GPIOA->BSRR = (1<<11);
        HAL_Delay(10);
        GPIOA->BRR = (1<<11);
        HAL_Delay(10);
    }
#endif

	/* Configure analog output pins -----------------------------------*/
	GPIO_InitStructure.Pin =  GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* DAC1 channel 1&2 Configuration for timer-triggered DMA */
	DacHandle.Instance = DAC1;
	HAL_DAC_Init(&DacHandle);

    /* hardware trigger from TIM2 */
    sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;

    /* in F303K8 DAC1 CHL1 has a buffer */
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_1);

    /* in F303K8 DAC1 CHL2 has a switch */
    sConfig.DAC_OutputSwitch = DAC_OUTPUTSWITCH_ENABLE;
	HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_2);

    /* enable DMA Request on Chl2 only - Chl1 will go along for the ride */
    SET_BIT(DacHandle.Instance->CR, DAC_CR_DMAEN2);

    /* turn on both DACS */
	__HAL_DAC_ENABLE(&DacHandle, DAC_CHANNEL_1);
	__HAL_DAC_ENABLE(&DacHandle, DAC_CHANNEL_2);

    /* Default initial output */
    DAC1->DHR12LD = (2048<<20) | (2048<<4);
    
	/* DAC2 channel 1 Configuration */
	DacHandle.Instance = DAC2;
	HAL_DAC_Init(&DacHandle);
    sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
    sConfig.DAC_OutputSwitch = DAC_OUTPUTSWITCH_ENABLE;
	HAL_DAC_ConfigChannel(&DacHandle, &sConfig, DAC_CHANNEL_1);
    //SET_BIT(DacHandle.Instance->CR, DAC_CR_TSEL1_2);    // force T2 trig
    SET_BIT(DacHandle.Instance->CR, DAC_CR_DMAEN1);   
	__HAL_DAC_ENABLE(&DacHandle, DAC_CHANNEL_1);
    DAC1->DHR12LD = 2048<<4;
    
    /* poke DMA remap for DACs to DMA1 */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_DMA_REMAP_CHANNEL_ENABLE(HAL_REMAPDMA_TIM7_DAC1_CH2_DMA1_CH4);
    __HAL_DMA_REMAP_CHANNEL_ENABLE(HAL_REMAPDMA_DAC2_CH1_DMA1_CH5);
    
    /* set up DMA for DAC1 chls 1&2 */
	__HAL_RCC_DMA1_CLK_ENABLE();
	DmaHandle.Instance                 = DMA1_Channel4;
	DmaHandle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
	DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
	DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
	DmaHandle.Init.Mode                = DMA_CIRCULAR;
	DmaHandle.Init.Priority            = DMA_PRIORITY_HIGH;
	
	/* Deinitialize the Stream for new transfer */
	HAL_DMA_DeInit(&DmaHandle);

	/* Configure the DMA Stream */
	HAL_DMA_Init(&DmaHandle);
  
	/* Configure the source, destination address and the data length */
	/* Configure DMA Stream data length */
	DmaHandle.Instance->CNDTR = (uint32_t)DAC_BUFSZ;

	/* Configure DMA Stream peripheral address */
	DmaHandle.Instance->CPAR = (uint32_t)&DAC1->DHR12LD;

	/* Configure DMA Stream memory address */
	DmaHandle.Instance->CMAR = (uint32_t)&dac1_buffer;

	/* Enable the RX half-transfer and transfer complete interrupts */
	__HAL_DMA_ENABLE_IT(&DmaHandle, DMA_IT_TC | DMA_IT_HT);

	/* DAC1 DMA IRQ Channel configuration */
	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	
	/* Enable DAC1 DMA */
	__HAL_DMA_ENABLE(&DmaHandle);
    
	/* set up DMA for DAC2 chl 1 */
	DmaHandle.Instance                 = DMA1_Channel5;
	// rest of struct is same as DAC1
    
	/* Deinitialize the Stream for new transfer */
	HAL_DMA_DeInit(&DmaHandle);

	/* Configure the DMA Stream */
	HAL_DMA_Init(&DmaHandle);
  
	/* Configure the source, destination address and the data length */
	/* Configure DMA Stream data length */
	DmaHandle.Instance->CNDTR = (uint32_t)DAC_BUFSZ;

	/* Configure DMA Stream peripheral address */
	DmaHandle.Instance->CPAR = (uint32_t)&DAC2->DHR12L1;

	/* Configure DMA Stream memory address */
	DmaHandle.Instance->CMAR = (uint32_t)&dac2_buffer;    
    
	/* Enable DAC2 DMA */
	__HAL_DMA_ENABLE(&DmaHandle);
    
	/* Enable TIM2 as timebase for DACs */
    __HAL_RCC_TIM2_CLK_ENABLE();
	TimHandle.Instance               = TIM2;
	TimHandle.Init.Prescaler         = 0;
	TimHandle.Init.Period            = 1332;	/* 48.012kHz */
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;
	TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&TimHandle);
    
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig);
}

/*
 * Start DACs
 */
void dac_start(void)
{
    HAL_TIM_Base_Start(&TimHandle);
}

/*
 * DMA1_Channel4 ISR handles all three DACs
 */
void DMA1_Channel4_IRQHandler(void)
{
	/* Raise activity flag */
	GPIOA->BSRR = (1<<11);

	/* measure timing */
	start_meas();

	/* Half-Transfer interrupt */
	if(DMA1->ISR&DMA_FLAG_HT4)
	{
		/* Clear the Interrupt flag */
		DMA1->IFCR = DMA_FLAG_HT4;

		/* load the first half of the buffers */
		audio((int16_t *)&dac1_buffer[0], 0);
		audio((int16_t *)&dac2_buffer[0], 2);
	}
	
	/* Transfer complete interrupt */
	if(DMA1->ISR&DMA_FLAG_TC4)
	{
		/* Clear the Interrupt flag */
		DMA1->IFCR = DMA_FLAG_TC4;
		
		/* load the 2nd half of the buffer */
		audio((int16_t *)&dac1_buffer[DAC_BUFSZ/2], 0);
		audio((int16_t *)&dac2_buffer[DAC_BUFSZ/2], 2);
	}
    
	/* measure timing */
	end_meas();

	/* Lower activity flag */
	GPIOA->BRR = (1<<11);
}
