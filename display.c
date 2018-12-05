#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

static short dispmem[] = {
        0x002,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x0c0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

//===========================================================================
// Send a command to the LCD.
void cmd(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = b;
}

//===========================================================================
// Send a character to the LCD.
void data(char b) {
    while((SPI2->SR & SPI_SR_TXE) != SPI_SR_TXE);
    SPI2->DR = 0x200 | b;
}

//===========================================================================
// Initialize the LCD.
void init_lcd_spi(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~0xcf000000;
    GPIOB->MODER |= 0x8a000000;

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI2->CR1 |= SPI_CR1_SPE;
    SPI2->CR1 &= ~SPI_CR1_BR;
    SPI2->CR1 |= 0x0018;

    nano_wait(100000000);
    cmd(0x38);
    cmd(0x0c);
    cmd(0x01);
    nano_wait(6200000);
    cmd(0x02);
    cmd(0x06);
}

//===========================================================================
// Initialize the LCD to use circular DMA to write to the SPI channel.
void init_lcd(void) {
    init_lcd_spi();

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    DMA1_Channel5->CMAR |= (uint32_t) dispmem;
    DMA1_Channel5->CPAR |= (uint32_t)  0x40003800 + 0x0c;
    DMA1_Channel5->CNDTR = 34;
    DMA1_Channel5->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel5->CCR |= 0x100;
    DMA1_Channel5->CCR &= ~DMA_CCR_MSIZE;
    DMA1_Channel5->CCR |= 0x400;
    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR &= ~DMA_CCR_PL;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;

    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

//===========================================================================
// Display a string on line 1 by writing to SPI directly.
void display1_spi(const char *s) {
    cmd(0x02); // put the cursor on the beginning of the first line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 2 by writing to SPI directly.
void display2_spi(const char *s) {
    cmd(0xc0); // put the cursor on the beginning of the second line.
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 1 by using DMA to write to SPI.
void display1_dma(const char *s) {
    short int temp;
    for(int i = 1; i < 17; i++){
        if(s[i-1] != '\0'){
            temp = s[i-1];
            temp |= 0x200;
            dispmem[i] = temp;
        }
        else dispmem[i] = 0x220;
    }

    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    DMA1_Channel5->CMAR |= (uint32_t) dispmem;
    DMA1_Channel5->CPAR |= (uint32_t)  0x40003800 + 0x0c;
    DMA1_Channel5->CNDTR = 17;
    DMA1_Channel5->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel5->CCR |= 0x100;
    DMA1_Channel5->CCR &= ~DMA_CCR_MSIZE;
    DMA1_Channel5->CCR |= 0x400;
    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR &= ~DMA_CCR_PL;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;

    SPI2->CR2 |= SPI_CR2_TXDMAEN;

    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

//===========================================================================
// Display a string on line 1 by writing to the DMA source.
void display1(const char *s) {
    short int temp;
    int is_done = 0;
    for(int i = 0; i < 16; i++){
        if(s[i] != '\0' && !is_done){
            temp = s[i];
            temp |= 0x200;
            dispmem[i+1] = temp;
        }
       else {
           is_done = 1;
           dispmem[i+1] = 0x220;
       }
    }
}

//===========================================================================
// Display a string on line 2 by writing to the DMA source.
void display2(const char *s) {
    short int temp;
    int is_done = 0;
    for(int i = 0; i < 16; i++){
        if(s[i] != '\0' && !is_done){
            temp = s[i];
            temp |= 0x200;
            dispmem[i+18] = temp;
        }
       else {
           is_done = 1;
           dispmem[i+18] = 0x220;
       }
    }
}

