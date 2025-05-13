#include <stdint.h>

static const uint32_t RCC = 0x40021000;         // RCC base address
static const uint32_t RCC_APB2ENR = RCC + 0x18; // RCC APB2 peripheral clock enable register

static const uint32_t GPIOA = 0x40010800;       // GPIOA base address
static const uint32_t GPIOA_CRH = GPIOA + 0x04; // GPIOA configuration register high

static const uint32_t GPIOC = 0x40011000;       // GPIOC base address
static const uint32_t GPIOC_CRH = GPIOC + 0x04; // GPIOC configuration register high
static const uint32_t GPIOC_ODR = GPIOC + 0x0C; // GPIOC output data register

static const uint32_t UART = 0x40013800;      // UART base address
static const uint32_t UART_SR = UART + 0x00;  // UART status register
static const uint32_t UART_DR = UART + 0x04;  // UART data register
static const uint32_t UART_BRR = UART + 0x08; // UART baud rate register
static const uint32_t UART_CR1 = UART + 0x0C; // UART control register 1
static const uint32_t UART_CR2 = UART + 0x10; // UART control register 2
static const uint32_t UART_CR3 = UART + 0x14; // UART control register 3

void main(void)
{
    // enable GPIO clock
    *(volatile uint32_t *)RCC_APB2ENR |= (1 << 2);  // enable GPIOA clock
    *(volatile uint32_t *)RCC_APB2ENR |= (1 << 4);  // enable GPIOC clock
    *(volatile uint32_t *)RCC_APB2ENR |= (1 << 14); // enable UART clock

    // define LED pin as output
    *(volatile uint32_t *)(GPIOC_CRH) = (*(uint32_t *)GPIOC_CRH & ~(0xF << 20)) | (0b10 << 20) | (0b00 << 22);

    // configure UART
    // configure pins for UART TX and RX to alternate function
    *(volatile uint32_t *)(GPIOA_CRH) = (*(uint32_t *)GPIOA_CRH & ~(0xF << 4)) | (0b10 << 4) | (0b10 << 6);  // PA9 (TX) as alternate function push-pull
    *(volatile uint32_t *)(GPIOA_CRH) = (*(uint32_t *)GPIOA_CRH & ~(0xF << 8)) | (0b10 << 8) | (0b11 << 10); // PA10 (RX) as input floating
    // enable UART
    *(volatile uint32_t *)UART_CR1 = 0;          // reset control register 1
    *(volatile uint32_t *)UART_CR1 |= (1 << 13); // UE (USART enable)
    // configure baud rate
    // baud = f_clock / (16 * (USARTDIV))
    // for 9600 baud with 8 MHz clock
    // USARTDIV = 8 MHz / (16 * 9600) = 52.0833
    // 0.0833 * 16 = 1.3333
    *(volatile uint32_t *)UART_BRR = 52 << 4 | 1;

    while (1)
    {
        // toggle LED
        *(volatile uint32_t *)GPIOC_ODR ^= (1 << 13); // toggle PC13

        // for each character in "hello world"
        const char *message = "hello world\n";

        // enable UART transmitter
        *(volatile uint32_t *)UART_CR1 |= (1 << 3); // TE (transmitter enable)

        for (const char *p = message; *p != '\0'; p++)
        {
            // wait until TXE (transmit data register empty) is set
            while (!(*(volatile uint32_t *)UART_SR & (1 << 7)))
                ;

            // send character
            *(volatile uint32_t *)UART_DR = *p;
        }

        // delay
        for (volatile int i = 0; i < 1000000; i++)
        {
            // simple delay loop
            __asm__ volatile("nop");
        }
    }
}

static const uint32_t _eram = 0x20004FFF; // end of RAM
extern uint32_t _sdata;                   // start of initialized data
extern uint32_t _edata;                   // end of initialized data
extern uint32_t _sbss;                    // start of uninitialized data
extern uint32_t _ebss;                    // end of uninitialized data
extern uint32_t _etext;                   // end of text section

void reset_handler(void)
{
    // initialize static uninitialized data
    for (uint32_t *p = (uint32_t *)_sbss; p < (uint32_t *)_ebss; p++)
    {
        *p = 0;
    }

    // initialize static initialized data
    uint32_t *init_values = (uint32_t *)_etext;
    for (uint32_t *p = (uint32_t *)_sdata; p < (uint32_t *)_edata; p++)
    {
        *p = *init_values++;
    }

    main();

    while (1)
    {
        // infinite loop
    }
}

uint32_t vector_table[128] __attribute__((section(".isr_vector"))) = {
    _eram,                   // initial stack pointer
    (uint32_t)reset_handler, // reset handler
};