#include <stdint.h>

static const uint32_t RCC = 0x40021000; // RCC base address
static const uint32_t RCC_APB2ENR = RCC+0x18; // RCC APB2 peripheral clock enable register

static const uint32_t GPIOC = 0x40011000; // GPIOC base address
static const uint32_t GPIOC_CRH = GPIOC+0x04; // GPIOC configuration register high
static const uint32_t GPIOC_ODR = GPIOC+0x0C; // GPIOC output data register

void main(void) {
    // enable GPIO clock
    *(volatile uint32_t *)RCC_APB2ENR |= (1 << 4); // enable GPIOC clock

    // define LED pin as output
    *(volatile uint32_t *)(GPIOC_CRH) = (*(uint32_t *)GPIOC_CRH & ~(0xF << 20)) | (0b10 << 20) | (0b00 << 22);

    while(1) {
        // toggle LED
        *(volatile uint32_t *)GPIOC_ODR ^= (1 << 13); // toggle PC13

        // delay
        for (volatile int i = 0; i < 1000000; i++) {
            // simple delay loop
            __asm__ volatile("nop");
        }
    }
}

static const uint32_t _eram = 0x20004FFF; // end of RAM
extern uint32_t _sdata; // start of initialized data
extern uint32_t _edata; // end of initialized data
extern uint32_t _sbss; // start of uninitialized data
extern uint32_t _ebss; // end of uninitialized data
extern uint32_t _etext; // end of text section

void reset_handler(void) {
    // initialize static uninitialized data
    for (uint32_t *p = (uint32_t *)_sbss; p < (uint32_t *)_ebss; p++) {
        *p = 0;
    }

    // initialize static initialized data
    uint32_t* init_values = (uint32_t *)_etext;
    for (uint32_t *p = (uint32_t *)_sdata; p < (uint32_t *)_edata; p++) {
        *p = *init_values++;
    }

    main();

    while(1) {
        // infinite loop
    }
}

uint32_t vector_table[128] __attribute__((section(".isr_vector"))) = {
    _eram, // initial stack pointer
    (uint32_t)reset_handler, // reset handler
};