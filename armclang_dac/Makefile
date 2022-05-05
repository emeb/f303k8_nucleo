# Makefile for STM32F303 w/ HAL
# 07-31-2019 E. Brombaugh

# sub directories
VPATH = .:../CMSIS:../HAL:../common

# Object files
OBJECTS =   startup_stm32f303x8_arm.o system_stm32f3xx.o main.o printf.o \
			usart.o led.o dac.o audio.o filter_mg4_v1.o cyclesleep.o \
            stm32f3xx_hal_gpio.o stm32f3xx_hal_rcc.o stm32f3xx_hal_cortex.o \
			stm32f3xx_hal.o stm32f3xx_hal_pwr_ex.o stm32f3xx_hal_uart.o \
            stm32f3xx_hal_rcc_ex.o stm32f3xx_hal_dac.o stm32f3xx_hal_tim.o \
			stm32f3xx_hal_tim_ex.o stm32f3xx_hal_dma.o stm32f3xx_hal_dac_ex.o \
			stm32f3xx_hal_uart_ex.o

# Linker script
LDSCRIPT = scatter.scat

# Compiler Flags
CFLAGS  = -g -O0 -std=gnu99 -Wall -flto
CFLAGS += -I. -I../CMSIS -I../HAL -I../common -I /home/ericb/developmentstudio-2022.0/sw/ARMCompiler6.18/include/
CFLAGS += -DARM_MATH_CM4 -DUSE_HAL_DRIVER -D'__FPU_USED=1'
CFLAGS += -DSTM32F303x8 -D'HSE_VALUE=((uint32_t)8000000)'
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 --target=arm-arm-none-eabi
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 --target=arm-arm-none-eabi -masm=auto -flto
LFLAGS  = --scatter=scatter.scat --info=totals --info=unused --lto
CPFLAGS = 

# Executables
CC = /home/ericb/developmentstudio-2022.0/sw/ARMCompiler6.18/bin/armclang
AS = $(CC)
LD = /home/ericb/developmentstudio-2022.0/sw/ARMCompiler6.18/bin/armlink
FE = fromelf
OBJDMP = arm-none-eabi-objdump
GDB = arm-none-eabi-gdb
OPENOCD = openocd

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) crt.lst *.lst *.elf *.bin *.map *.dmp

#flash: gdb_flash
flash: oocd_flash

oocd_flash_bin: main.bin
	$(OPENOCD) -f openocd_stlink.cfg -c "program main.bin verify reset exit 0x08000000"

oocd_flash: main_gnu.elf
	$(OPENOCD) -f openocd_stlink.cfg -c "program main_gnu.elf verify reset exit"

disassemble: main.elf
	$(OBJDMP) -d main_gnu.elf > main.dis
	
dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > minimal_hello_world.tar.gz

main.bin: main.elf 
	$(FE) --bin main.elf --output main.bin
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(info ------Linking------)
	$(LD) $(LFLAGS) -o main.elf $(OBJECTS)
	$(FE) --elf main.elf --output main_gnu.elf

startup_stm32f303x8_arm.o: ../CMSIS/startup_stm32f303x8_arm.s
	$(AS) $(AFLAGS) -c -o $@ ../CMSIS/startup_stm32f303x8_arm.s 	

%.o: %.c %.h %.s
	$(CC) $(CFLAGS) -c -o $@ $<

