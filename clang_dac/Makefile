# Makefile for STM32F303 w/ HAL
# 07-31-2019 E. Brombaugh

# sub directories
VPATH = .:../CMSIS:../HAL:../common

# Object files
OBJECTS =   startup_stm32f303x8.o system_stm32f3xx.o main.o printf.o \
			usart.o led.o dac.o audio.o filter_mg4_v1.o cyclesleep.o \
            stm32f3xx_hal_gpio.o stm32f3xx_hal_rcc.o stm32f3xx_hal_cortex.o \
			stm32f3xx_hal.o stm32f3xx_hal_pwr_ex.o stm32f3xx_hal_uart.o \
            stm32f3xx_hal_rcc_ex.o stm32f3xx_hal_dac.o stm32f3xx_hal_tim.o \
			stm32f3xx_hal_tim_ex.o stm32f3xx_hal_dma.o stm32f3xx_hal_dac_ex.o 

# Linker script
LDSCRIPT = STM32F303K8Tx_FLASH.ld

# Need base GCC architecture stuff for both gcc and clang
ARCH = /opt/launchpad/gcc-arm-none-eabi-7-2018-q2-update/bin/arm-none-eabi
#ARCH = /opt/launchpad/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin/arm-none-eabi
ARM_GNU_CC = $(ARCH)-gcc
ARCH_CFLAGS = -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_CFLAGS += -mlittle-endian -mthumb
ARCH_LFLAGS = $(ARCH_CFLAGS) -T $(LDSCRIPT) -Wl,-Map=main.map -Wl,--gc-sections

# set which version of clang
CLANG = /usr/bin/clang

# Compiler Flags
CFLAGS  = -g -Os -ffunction-sections -std=gnu99 -Wall -flto
CFLAGS += -I. -I../CMSIS -I../HAL -I../common
CFLAGS += -DARM_MATH_CM4 -DUSE_HAL_DRIVER
CFLAGS += -DSTM32F303x8 -D'HSE_VALUE=((uint32_t)8000000)'
CFLAGS += $(ARCH_CFLAGS)
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4
LFLAGS  = $(CFLAGS) -T $(LDSCRIPT) -Wl,-Map=main.map -Wl,--gc-sections
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Pick a compiler
COMPILER = $(CLANG)
#COMPILER = $(ARM_GNU_CC)
CC = $(COMPILER)

# Clang stuff
ifneq '' '$(findstring clang,$(CC))'
$(info ===Clang Compiler Detected===)

# location of gcc install
ARM_CORTEXM_SYSROOT = \
  $(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-sysroot 2>&1)
$(info --->sysroot is $(ARM_CORTEXM_SYSROOT))

# The directory where Newlib's libc.a & libm.a reside
# for the specific target architecture
ARM_CORTEXM_MULTI_DIR = \
  $(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-multi-directory 2>&1)
$(info --->multi-dir is $(ARM_CORTEXM_MULTI_DIR))

# Pick up builtins needed for compilation
ARM_CORTEXM_BUILTINS ?= \
$(shell $(ARM_GNU_CC) $(ARCH_CFLAGS) -print-libgcc-file-name 2>&1)
$(info --->builtins is $(ARM_CORTEXM_BUILTINS))

COMPILER_SPECIFIC_CFLAGS = \
  --target=arm-arm-none-eabi \
  --sysroot=$(ARM_CORTEXM_SYSROOT)

COMPILER_SPECIFIC_LDFLAGS = -nostdlib -lc \
  -L$(ARM_CORTEXM_SYSROOT)/lib/$(ARM_CORTEXM_MULTI_DIR) \
  $(ARM_CORTEXM_BUILTINS)

#LD = ld.lld
AS = $(CLANG) $(CFLAGS) -c

endif

# GCC stuff
ifneq '' '$(findstring gcc,$(CC))'
$(info ===GCC Compiler Detected===)

LD = $(ARCH)-ld -v
AS = $(ARCH)-as

COMPILER_SPECIFIC_CFLAGS +=
COMPILER_SPECIFIC_LDFLAGS += -Wl,--print-memory-usage --specs=nano.specs -nostartfiles 

endif

$(info --->compiler specific cflags is $(COMPILER_SPECIFIC_CFLAGS))
$(info --->cflags is $(CFLAGS))
CFLAGS += $(COMPILER_SPECIFIC_CFLAGS)
LFLAGS += $(COMPILER_SPECIFIC_LDFLAGS)
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Other Executables
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
OPENOCD = openocd

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) crt.lst *.lst *.elf *.bin *.map *.dmp

#flash: gdb_flash
flash: oocd_flash

oocd_flash: main.elf
	$(OPENOCD) -f openocd_stlink.cfg -c "program main.elf verify reset exit"

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
	$(info ------Linking------)
	$(CC) $(LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm
	#$(ARM_GNU_CC) $(ARCH_LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm

%.o: %.c %.h %.s
	$(CC) $(CFLAGS) -c -o $@ $<

