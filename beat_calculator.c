#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
void display1(const char *s);
void display2(const char *s);

int last4[4] = {60000,60000,60000,60000};
int time_since = 0;
int current_pos = 0;
int BPM = 0;
char line[17];

void calculate_BPM(void){
    int sum = 0;

    for(int i = 0; i < 4; i++) {
        sum += last4[i];
    }
    BPM = 60000/(sum>>2);

    sprintf(line, "TEMPO: %d", BPM);
    display1(line);
}


void INSERT_NAME_OF_INTERRUPT(void){
    last4[current_pos++] = time_since;

    time_since = 0;

    calculate_BPM();

    if(current_pos > 3){
        current_pos = 0;
    }

    int useless;
    useless = TIM2->CCR1;
    useless = TIM2->CCR2;
}

/* TIM3_IRQHandler
 *
 * Updates the count since last
 */
void TIM3_IRQHandler(void){
    time_since++;
    TIM3->SR &= ~TIM_SR_UIF;
}

/* tim3_init
 *
 * Initializes the timer 3 subsystem to trigger an
 * interrupt every 1ms.
 */
void tim3_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->CR1 &= ~TIM_CR1_DIR;

    TIM3->PSC = 1-1;
    TIM3->ARR = 48000-1;

    TIM3->DIER |= TIM_DIER_UIE;

    TIM3->CR1 |= TIM_CR1_CEN;

    NVIC->ISER[0] = 1<<TIM3_IRQn;
}
