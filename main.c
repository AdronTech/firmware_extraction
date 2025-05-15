#include <stdint.h>

static const uint32_t RCC = 0x40021000;        // RCC base address
static const uint32_t RCC_AHBEN = RCC + 0x14;  // RCC AHB enable register
static const uint32_t RCC_APB1EN = RCC + 0x1C; // RCC APB2 enable register

static const uint32_t GPIOA = 0x48000000;          // GPIOA base address
static const uint32_t GPIOA_CTL = GPIOA + 0x00;    // GPIOA configuration register high
static const uint32_t GPIOA_ODR = GPIOA + 0x14;    // GPIOA output data register
static const uint32_t GPIOA_AFSEL0 = GPIOA + 0x20; // GPIOA alternate function low register

static const uint32_t UART = 0x40004400; // UART base address
// static const uint32_t UART_SR = UART + 0x00;  // UART status register
// static const uint32_t UART_DR = UART + 0x04;  // UART data register
// static const uint32_t UART_BRR = UART + 0x08; // UART baud rate register
// static const uint32_t UART_CR1 = UART + 0x0C; // UART control register 1
// static const uint32_t UART_CR2 = UART + 0x10; // UART control register 2
// static const uint32_t UART_CR3 = UART + 0x14; // UART control register 3
static const uint32_t USART_CTL0 = UART + 0x00;  // UART control register 0
static const uint32_t USART_CTL1 = UART + 0x04;  // UART control register 1
static const uint32_t USART_CTL2 = UART + 0x08;  // UART control register 2
static const uint32_t USART_BAUD = UART + 0x0C;  // UART baud rate register
static const uint32_t USART_STAT = UART + 0x1C;  // UART status register
static const uint32_t USART_TDATA = UART + 0x28; // UART data register

static const uint32_t FLASH_START = 0x08000000; // FLASH start address
static const uint32_t FLASH_SIZE = 0x00010000;  // FLASH size (64 KB)

void init_led(void)
{
    // enable GPIO clock
    *(volatile uint32_t *)RCC_AHBEN |= (1 << 17); // enable GPIOA clock

    // define LED pin as output
    *(volatile uint32_t *)(GPIOA_CTL) |= (0b01 << 4); // PA2 as output push-pull
}

void toggle_led(void)
{
    *(volatile uint32_t *)GPIOA_ODR ^= (1 << 2); // toggle PA2
}

void init_uart(void)
{
    // enable UART clock
    *(volatile uint32_t *)RCC_APB1EN |= (1 << 17); // enable UART clock

    // configure pins for UART TX and RX to alternate function
    *(volatile uint32_t *)RCC_AHBEN |= (1 << 17);          // enable GPIOA clock
    *(volatile uint32_t *)(GPIOA_CTL) |= (0b10 << 4);      // PA2 (TX) as alternate function
    *(volatile uint32_t *)(GPIOA_AFSEL0) |= (0b0001 << 8); // PA2 alternate function 1 (USART1)

    // configure baud rate
    // baud = f_clock / (16 * (USARTDIV))
    // for 250.000 baud with 8 MHz clock
    // USARTDIV = 8 MHz / (16 * 250.000) = 2
    // 0 * 16 = 0
    *(volatile uint32_t *)USART_BAUD = 2 << 4 | 0;

    // enable UART
    *(volatile uint32_t *)USART_CTL0 |= (1 << 0); // UE (USART enable)
}

void send_byte_sync(uint8_t byte)
{
    // wait until TXE (transmit data register empty) is set
    while (!(*(volatile uint32_t *)USART_STAT & (1 << 7)))
        ;

    // send byte
    *(volatile uint32_t *)USART_TDATA = byte;
}

void main(void)
{
    // init_led();
    init_uart();

    while (1)
    {
        // toggle_led();

        // for each character in "hello world"
        const char *message = "hello world\n";

        // enable UART transmitter
        *(volatile uint32_t *)USART_CTL0 |= (1 << 3); // TE (transmitter enable)

        for (const char *p = message; *p != '\0'; p++)
        {
            send_byte_sync(*p); // send character
        }

        // // dump flash memory
        // for (uint32_t addr = FLASH_START; addr < FLASH_START + FLASH_SIZE; addr += 4)
        // {
        //     send_byte_sync((*((uint32_t *)addr) >> 24) & 0xFF); // MSB
        //     send_byte_sync((*((uint32_t *)addr) >> 16) & 0xFF);
        //     send_byte_sync((*((uint32_t *)addr) >> 8) & 0xFF);
        //     send_byte_sync((*((uint32_t *)addr)) & 0xFF); // LSB
        // }

        // delay
        for (volatile int i = 0; i < 100000; i++)
        {
            // simple delay loop
            __asm__ volatile("nop");
        }
    }
}

extern uint32_t _eram;  // end of RAM
extern uint32_t _sdata; // start of initialized data
extern uint32_t _edata; // end of initialized data
extern uint32_t _sbss;  // start of uninitialized data
extern uint32_t _ebss;  // end of uninitialized data
extern uint32_t _etext; // end of text section

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
    (uint32_t)&_eram,        // initial stack pointer
    (uint32_t)reset_handler, // reset handler
};