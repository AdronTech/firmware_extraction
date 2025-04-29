# Set vector table to RAM
set *(unsigned int*)0xE000ED08 = 0x20000000

# Set Stack Pointer
set $sp = *(unsigned int*)0x20000000

# Set Program Counter
set $pc = *(unsigned int*)0x20000004

# Start execution
continue
