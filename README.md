# STM32F1 "Blue Pill" GPIO Blinky
This is a simple GPIO blinky program for the STM32F1 "Blue Pill" board. It toggles the onboard LED on pin PC13.

## Prerequisites
- STM32F1 "Blue Pill" board
- OpenOCD
- ARM toolchain (e.g., `arm-none-eabi-gcc`)
- GDB (GNU Debugger)
- A flashing tool (e.g., Rpi Debug Probe, ST-Link, etc.)
  - programmer has to be configured in `openocd.cfg` file

## flash version
### building
To build the flash version of the program, use the following command:
```
arm-none-eabi-gcc main.c -T memory.ld -o blink.elf -mcpu=cortex-m3 -mthumb -nostdlib -g
```
This command compiles `main.c` using the `memory.ld` linker script, targeting the Cortex-M3 processor with Thumb instructions. The `-nostdlib` flag ensures no standard libraries are linked, and `-g` includes debugging information.

### flashing
To flash the program onto the STM32F1 "Blue Pill" board, run:
```
openocd -f openocd.cfg -c "program blink.elf verify reset exit"
```
This command uses OpenOCD to program the `blink.elf` file, verify the flash, reset the board, and then exit.

### debugging
To debug the flash version, start OpenOCD:
```
openocd -f openocd.cfg
```
Then, in another terminal, start GDB:
```
arm-none-eabi-gdb blink.elf
```
Connect to the target and load the program:
```
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
```
This sequence connects GDB to the OpenOCD server, halts the target, and loads the program for debugging.

## ram version
### building
To build the RAM version of the program, use the following command:
```
arm-none-eabi-gcc main.c -T ram_memory.ld -o ram_blink.elf -mcpu=cortex-m3 -mthumb -nostdlib -g
```
This command is similar to the flash version but uses the `ram_memory.ld` linker script to place the program in RAM.

### flashing and running
Start OpenOCD:
```
openocd -f openocd.cfg
```
Then, in another terminal, start GDB:
```
arm-none-eabi-gdb ram_blink.elf
```
Connect to the target, load the program, and execute the RAM boot script:
```
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) source ram_boot.gdb
```
The `ram_boot.gdb` script sets up the vector table, stack pointer, and program counter for execution from RAM.

