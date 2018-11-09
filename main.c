/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"


int main(void)
{
	/* DATA CAPTURE SETUP CODE */

	init_ADC();
	init_DMA();

	//Enable ADC
	ADC1->CR |= ADC_CR_ADEN;

	//Ready ADC
	while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
	while(!(ADC1->ISR & ADC_ISR_ADRDY));
	while((ADC1->CR & ADC_CR_ADSTART));

	//Enable DMA
	DMA1_Channel1->CCR |= DMA_CCR_EN;

	//Fire it up!
	ADC1->CR |= ADC_CR_ADSTART;

	for(;;)
	{
		asm("wfi");
	}
}
