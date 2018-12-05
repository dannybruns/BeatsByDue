#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void en_interrupt(void) {

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1);
	EXTI->FTSR |= EXTI_FTSR_TR0;
	EXTI->IMR |= EXTI_IMR_MR0;
	NVIC->ISER[0] |= 1 << EXTI0_1_IRQn;

	return;
}

