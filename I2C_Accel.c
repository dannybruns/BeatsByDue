#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

int8_t i2c_senddata(uint8_t, void*, uint8_t);
int8_t i2c_recvdata(uint8_t, void*, uint8_t);

void init_I2C(void)
{
	/* Enable PB6 & PB7 as SCL & SDA */
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER |= 2<<(2*6) | 2<<(2*7);
    GPIOB->AFR[0] |= 1<<(4*6) | 1<<(4*7);

    /* I2C1 Clk Enable */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /* I2C CR1 Config */
    I2C1->CR1 &= ~I2C_CR1_PE;           // Disable to perform reset.
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;       // 0: Analog noise filter enabled.
    I2C1->CR1 &= ~I2C_CR1_ERRIE;        // Errors interrupt disable
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;    // Enable clock stretching

    /* I2C1 Timing Settings (100kHz & 8MHz Clock) Based on P.642 of FRM */
    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;
    I2C1->TIMINGR |= 0x10000000;	//PRESC = 1
    I2C1->TIMINGR |= 0x00000013;	//SCLL = 0x13
    I2C1->TIMINGR |= 0x00000F00;	//SCLH = 0x0F
    I2C1->TIMINGR |= 0x00020000;	//SDADEL = 0x2
    I2C1->TIMINGR |= 0x00400000;	//SCLDEL = 0x4

    /* I2C Own address 1 register (I2C_OAR1) */
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 =   I2C_OAR1_OA1EN | (0x1<<1);// Set 7-bit own address 1

    /* I2C1 CR2 Config */
    I2C1->CR2 &= ~I2C_CR2_ADD10;        // 0 = 7-bit mode; 1 = 10-bit mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;       // Enable the auto end
    I2C1->CR2 |= I2C_CR2_NACK;          // For slave mode: set NACK

    /* I2C1 Peripheral Enable */
    I2C1->CR1 |= I2C_CR1_PE;            // Enable I2C1

    return;
}

void init_MMA8452Accel(void)
{
	//Enable Single Tap Only, Normal Mode, 400 Hz ODR

	/* 400 Hz, Standby Mode */
	uint8_t cr_1[] = {0x2A, 0x08};
	i2c_senddata(0x1D, cr_1, sizeof cr_1);

	/* Enable X, Y, Z Single Pulse */
	uint8_t pulse_cfg[] = {0x21, 0x15};
	i2c_senddata(0x1D, pulse_cfg, sizeof pulse_cfg);

	/* Set X, Y, Z Thresholds to 1.575, 1.575, 2.65g Respectively */
	uint8_t pulse_thsx[] = {0x23, 0x19};
	i2c_senddata(0x1D, pulse_thsx, sizeof pulse_thsx);

	uint8_t pulse_thsy[] = {0x24, 0x19};
	i2c_senddata(0x1D, pulse_thsy, sizeof pulse_thsy);

	uint8_t pulse_thsz[] = {0x25, 0x2A};
	i2c_senddata(0x1D, pulse_thsz, sizeof pulse_thsz);

	/* Set time limit for tap detection to 50 ms, normal mode, no lpf */
	/* Data Rate = 400 Hz, Time Step = 0.625 ms */
	uint8_t pulse_tmlt[] = {0x26, 0x50};
	i2c_senddata(0x1D, pulse_tmlt, sizeof pulse_tmlt);

	/* Set latency time to 300 ms */
	uint8_t pulse_ltcy[] = {0x27, 0xf0};
	i2c_senddata(0x1D, pulse_tmlt, sizeof pulse_tmlt);

	/* Route INT1 to System Interrupt */
	uint8_t cr_4[] = {0x2D, 0x08};
	i2c_senddata(0x1D, cr_4, sizeof cr_4);

	uint8_t cr_5[] = {0x2E, 0x08};
	i2c_senddata(0x1D, cr_5, sizeof cr_5);

	/* Put Device in Active Mode */
	uint8_t temp[] = {0x2A};
	i2c_recvdata(0x1D, temp, sizeof temp);

	/* temp is now equal to the value at 0x2A */
	temp[0] |= 0x01;
	uint8_t cmd[] = {0x2A, temp[0]};
	i2c_senddata(0x1D, cmd, sizeof cmd);


	return;
}

void init_MMA8452_conduct(void)
{
	/* 400 Hz, Standby Mode */
	uint8_t cr_1[] = {0x2A, 0x08};
	i2c_senddata(0x1D, cr_1, sizeof cr_1);

	/* Enable X, Y, Z Single Pulse */
	uint8_t pulse_cfg[] = {0x21, 0x15};
	i2c_senddata(0x1D, pulse_cfg, sizeof pulse_cfg);

	/* Set X, Y, Z Thresholds to 1.575, 1.575, 1.575g Respectively */
	uint8_t pulse_thsx[] = {0x23, 0x19};
	i2c_senddata(0x1D, pulse_thsx, sizeof pulse_thsx);

	uint8_t pulse_thsy[] = {0x24, 0x19};
	i2c_senddata(0x1D, pulse_thsy, sizeof pulse_thsy);

	uint8_t pulse_thsz[] = {0x25, 0x19};
	i2c_senddata(0x1D, pulse_thsz, sizeof pulse_thsz);

	/* Set time limit for tap detection to 50 ms, normal mode, no lpf */
	/* Data Rate = 800 Hz, Time Step = ?? ms */
	uint8_t pulse_tmlt[] = {0x26, 0x50};
	i2c_senddata(0x1D, pulse_tmlt, sizeof pulse_tmlt);

	/* Set latency time to ?? ms */
	uint8_t pulse_ltcy[] = {0x27, 0x02};
	i2c_senddata(0x1D, pulse_tmlt, sizeof pulse_tmlt);

	/* Route INT1 to System Interrupt */
	uint8_t cr_4[] = {0x2D, 0x08};
	i2c_senddata(0x1D, cr_4, sizeof cr_4);

	uint8_t cr_5[] = {0x2E, 0x08};
	i2c_senddata(0x1D, cr_5, sizeof cr_5);

	/* Put Device in Active Mode */
	uint8_t temp[] = {0x2A};
	i2c_recvdata(0x1D, temp, sizeof temp);

	/* temp is now equal to the value at 0x2A */
	temp[0] |= 0x01;
	uint8_t cmd[] = {0x2A, temp[0]};
	i2c_senddata(0x1D, cmd, sizeof cmd);

	return;
}

//===========================================================================
// Clear NACK condition.
int i2c_checknack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        return 1;
    return 0;
}

//===========================================================================
// Clear NACK condition.
void i2c_clearnack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        I2C1->ICR |= I2C_ICR_NACKCF;
}

//===========================================================================
// Clear STOP condition.
int i2c_checkstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return 1;
    return 0;
}

//===========================================================================
// Clear STOP condition.
void i2c_clearstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        I2C1->ICR |= I2C_ICR_STOPCF;
}

//===========================================================================
// Look at the ISR and display the error.
void display_i2c_error(void) {
    if (I2C1->ISR & I2C_ISR_BERR)
        display1("Bus error");
    else if (I2C1->ISR & I2C_ISR_ARLO)
        display1("Arbitration Lost");
    else if (I2C1->ISR & I2C_ISR_TIMEOUT)
        display1("I2C TIMEOUT");
    else if (I2C1->ISR & I2C_ISR_BUSY)
        display1("I2C BUSY");
    else if (I2C1->ISR & I2C_ISR_PECERR)
        display1("I2C PEC Error");
    else if (I2C1->ISR & I2C_ISR_STOPF)
        display1("I2C Stopped");
    else if (I2C1->ISR & I2C_ISR_NACKF)
        display1("I2C NACK");
    else
        display1("???");
}

//===========================================================================
// Generate a start bit.
void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir) {
    // dir: 0 = master requests a write transfer
    // dir: 1 = master requests a read transfer
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES |
                I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN;  // Read from slave
    else
        tmpreg &= I2C_CR2_RD_WRN;  // Write to slave
    tmpreg |= ((devaddr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

//===========================================================================
// Generate a stop bit.
void i2c_stop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return;
    // Master: Generate STOP bit after current byte has been transferred.
    I2C1->CR2 |= I2C_CR2_STOP;
    // Wait until STOPF flag is reset
    while( (I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF; // Write  to clear STOPF flag
}

//===========================================================================
// Check wait for the bus to be idle.
void i2c_waitidle(void) {
    while ( (I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

//===========================================================================
// Send multiple bytes to a target slave.
// - Wait for idle.
// - Send data, byte by byte.
// - After all bytes sent, hardware sets the TC flag.
//   Software waits until TC is set.
//   Hardware automatically clears TC flag when START bit or STOP bit in
//   CR2 is set.
int8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size) {
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    // Last argument is dir: 0 = sending data to the slave.
    i2c_start(devaddr, size, 0);

    for(i=0; i<size; i++) {
        // TXIS bit is set by hardware when the TXDR register is empty and the
        // data to be transmitted must be written in the TXDR register.  It is
        // cleared when the next data to be sent is written in the TXDR reg.
        // The TXIS flag is not set when a NACK is received.
        int count = 0;
        while( (I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }

        // TXIS is cleared by writing to the TXDR register.
        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    // Wait until TC flag is set or the NACK flag is set.
    while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ( (I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}

//===========================================================================
// Receive multiple bytes from a target slave.
int8_t i2c_recvdata(uint8_t devaddr, void *pdata, uint8_t size) {
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    i2c_start(devaddr, size, 1); // 1 = receiving from the slave
    for(i=0; i<size; i++) {
        // Wait until RXNE flag is set
        int count = 0;
        while ( (I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        udata[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
    }

    int x=0;
    while((I2C1->ISR & I2C_ISR_TC) == 0)
        x++;  // Wait until TCR flag is set

    i2c_stop();
    return 0;
}


