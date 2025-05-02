#include "stm32f0xx.h"

extern void nano_wait(unsigned int n);

void init_uart() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_USART2EN;
    GPIOA -> MODER |= 0xA << 4;
    GPIOA -> AFR[0] |= 0x11 << 8;

    USART2 -> CR1 &= ~USART_CR1_UE;
    USART2 -> CR1 &= ~(0x1 << 28);
    USART2 -> CR1 &= ~(0x1 << 12);
    USART2 -> CR2 &= ~(0x3 << 12);
    USART2 -> CR1 &= ~(0x1 << 10);
    USART2 -> CR1 &= ~(0x1 << 15);
    USART2 -> BRR = 500;
    USART2 -> CR1 |= USART_CR1_TE;
    USART2 -> CR1 |= USART_CR1_RE;
    USART2 -> CR1 |= USART_CR1_UE;

    while ((USART2->ISR & (USART_ISR_TEACK | USART_ISR_REACK)) != 
       (USART_ISR_TEACK | USART_ISR_REACK));
}

void test(int isPlayerOne) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(0xFF << 12);
    GPIOC->MODER |= 0x55 << 12;
    
    init_uart();
    

    if(isPlayerOne) {
        volatile char send = 0xAA;
        nano_wait(10000000); // Let UART stabilize
        while (!(USART2->ISR & USART_ISR_TXE));
        USART2->TDR = send;
        while (!(USART2->ISR & USART_ISR_TC));
        GPIOC -> ODR |= GPIO_ODR_6;
    } else {
        volatile char receive;
        while(!(USART2->ISR & USART_ISR_RXNE));
        receive = USART2->RDR;
        if(receive == 0xaa) {
            GPIOC -> ODR |= GPIO_ODR_6;
        } else {
            GPIOC -> ODR |= GPIO_ODR_9;
        }
    }
}
