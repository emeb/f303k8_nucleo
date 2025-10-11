# Makefile for STM32F303 w/ HAL
# 07-31-2019 E. Brombaugh

# sub directories
VPATH = .:../CMSIS:../HAL:../common

# Object files
OBJECTS =   startup_stm32f303x8.o system_stm32f3xx.o main.o printf.o \
			usart.o led.o \
            stm32f3xx_hal_gpio.o stm32f3xx_hal_rcc.o stm32f3xx_hal_cortex.o \
			stm32f3xx_hal.o stm32f3xx_hal_pwr_ex.o stm32f3xx_hal_uart.o \
            stm32f3xx_hal_rcc_ex.o stm32f3xx_hal_spi.o stm32f3xx_hal_dma.o
			
# Linker script
LDSCRIPT = STM32F303K8Tx_FLASH.ld

# Compiler Flags
CFLAGS  = -g -O3 -ffunction-sections -std=gnu99 -Wall -flto
CFLAGS += -I. -I../CMSIS -I../HAL -I../common
CFLAGS += -DARM_MATH_CM4 -DUSE_HAL_DRIVER
CFLAGS += -DSTM32F303x8 -D'HSE_VALUE=((uint32_t)8000000)'
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -mlittle-endian -mthumb
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4
LFLAGS  = $(CFLAGS) -nostartfiles -T $(LDSCRIPT) -Wl,-Map=main.map
LFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LFLAGS += --specs=nano.specs
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Executables
#ARCH = arm-none-eabi
ARCH = /opt/launchpad/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
OPENOCD = openocd

CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) crt.lst *.lst *.elf *.bin *.map *.dmp

#flash: gdb_flash
flash: oocd_flash

oocd_flash: main.elf
	$(OPENOCD) -f openocd_stlinkv2.1.cfg -c "program main.elf verify reset exit"

gdb_flash: main.elf
	$(GDB) -x flash_cmd.gdb -batch
	stty sane

disassemble: main.elf
	$(OBJDMP) -d main.elf > main.dis
	
dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > minimal_hello_world.tar.gz

main.ihex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.ihex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

