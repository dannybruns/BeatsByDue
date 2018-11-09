#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

static short accel[300] = {0};	//Circular Buffer
								//accel[0] is X value
								//accel[1] is Y value
								//accel[2] is Z value
								//accel[3] is X value
								//...

void init_ADC(void);
void init_DMA(void);
void ADC1_COMP_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);

/*************************************************
 *
 * 	init_ADC
 *
 * 	Accelerometer will be continuously reading from PA4, PA5, PA6 at a rate of 14 MHz
 *	PA4 -> X accel
 *	PA5 -> Y accel
 *	PA6 -> Z accel
 *
 *  NOTES:  ON reset, ADC is automatically operating in 12-bit right aligned mode
 *
 *************************************************/
void init_ADC(void)
{
	/* GPIO Config */

	//Enable PA4, PA5, PA6 as analog pins
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= (0xffffc0ff);
	GPIOA->MODER |= (0x00003f00);

	/* ADC Config */

	//Enable clock sources
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->CR2 |= RCC_CR2_HSI14ON;

	//Enable DMA Transfers upon new value calculations
	//Enable DMA operation in circular mode
	ADC1->CFGR1 |= ADC_CFGR1_DMAEN | ADC_CFGR1_DMACFG;

	//Enable interrupt generation (ADC End of Calculation)
	ADC1->IER |= ADC_IER_EOCIE;		//Note: must clear ADC1->ISR, EOC bit
	NVIC->ISER[0] |= (1 << ADC1_COMP_IRQn);

	//Initialize ADC to Channel 4, PA4
	ADC1->CHSELR = 0;
	ADC1->CHSELR |= ADC_CHSELR_CHSEL4;

	return;
}

/*************************************************
 *
 * 	init_DMA
 *
 * 	Initialize DMA channel 1 to send acceleration
 * 	readings directly into int[] accel.
 *
 *************************************************/
void init_DMA(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;	//'01' indicates 16 bit data being read
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;	//16 bit peripheral data

	DMA1_Channel1->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC;	//Circular mode, increment memory
	DMA1_Channel1->CNDTR = sizeof accel;				//Counts for the number of elements in accel

	DMA1_Channel1->CMAR = (uint32_t) accel;				//memory address points to accel
	DMA1_Channel1->CPAR = (uint32_t) 0x40012440;		//ADC address, offset by ADC_DR;

	// Uncomment to enable interrupt generation at the end of each transfer
	//DMA1_Channel1->CCR |= DMA_CCR_TCIE;	//Note: must set DMA_IFCR_CTCIF to '1'
	//NVIC->ISER[0] |= (1 << DMA1_Channel1_IRQn);

	return;
}

void ADC1_COMP_IRQHandler(void)
{
	if(ADC1->CHSELR & ADC_CHSELR_CHSEL4)
	{
		//If Channel 4 finished, switch to channel 5
		ADC1->CHSELR = 0;
		ADC1->CHSELR |= ADC_CHSELR_CHSEL5;
	}
	else if(ADC1->CHSELR & ADC_CHSELR_CHSEL5)
	{
		//If Channel 5 finished, switch to channel 6
		ADC1->CHSELR = 0;
		ADC1->CHSELR |= ADC_CHSELR_CHSEL6;
	}
	else if(ADC1->CHSELR & ADC_CHSELR_CHSEL6)
	{
		//If Channel 6 finished, switch to channel 4
		ADC1->CHSELR = 0;
		ADC1->CHSELR |= ADC_CHSELR_CHSEL4;
	}
	else
	{
		//If it is not CH 4, 5, or 6, Stop calculations
		ADC1->CHSELR = 0;
		//This SHOULD never happen.
	}

	//Beat detection should go here?

	ADC1->ISR &= ~(ADC_ISR_EOC);	//Acknowledge the Interrupt
	ADC1->CR |= ADC_CR_ADSTART;		//Fire up the next conversion!
	return;
}

void DMA1_Channel1_IRQHandler(void)
{
	//This function is just here to make sure that ADC values are making it into the array

	DMA1->IFCR |= DMA_IFCR_CTCIF1;	//Acknowledge the Interrupt
	return;
}
