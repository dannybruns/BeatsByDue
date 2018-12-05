/**
  ******************************************************************************
  * @file    main.c
  * @author  My Name Jeff
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include <stdlib.h> // for strtoul()

int main(void)
{
	init_lcd();					//Begin SPI communication to the LCD Display
	tim3_init();				//Begin counting on tim3
	en_interrupt();				//Enable external interrupt on PA0
	tim6_init();				//Begin counting on tim6
	dac_init();					//Enable DAC
	init_I2C();					//Initialize I2C communication with the accelerometer
	init_MMA8452_conduct();		//Set control registers of the MMA8452 Accelerometer

	for(;;)
	{
		asm("wfi");				//Wait for an interrupt
	}
}




