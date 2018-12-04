void en_interrupt(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN
	GPIOB->MODER &= ~(GPIO_MODER_MODER0_0 | GPIO_MODER_MODER0_1);
	EXTI->RTSR |= EXTI_RTSR_TR0;
	EXTI->MSR |= EXTI_IMR_MR0;
	NVIC->ISER |= 1 << EXTI0_1_IRQn;
}



void EXTI0_1_IRQHandler(void) {
}