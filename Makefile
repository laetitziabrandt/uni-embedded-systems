.DEFAULT_GOAL = all

CC = arm-none-eabi-gcc
CFLAGS += -g -O0 \
		  -nostdlib \
		  -ffunction-sections \
		  -fdata-sections \
          -ffreestanding \
		  -flto \
		  -Wall \
		  -mcpu=cortex-m3 \
		  -I include

LD = $(CC)
LDFLAGS = $(CFLAGS) -Wl,--gc-sections -T linker.ld
LDLIBS = -lgcc

sources = src/start.c \
		  src/exception.c \
		  src/memory.c \
		  src/snprintf.c \
		  src/main.c \
		  src/serial.c \
		  src/state.c \
		  src/menu.c \
		  src/clock.c \
		  src/sonic.c \
		  src/led.c \
		  src/motion.c \
		  src/drive.c \
		  src/matrix.c
		  

build/%.d: src/%.c Makefile linker.ld
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MM -MT $(@:.d=.o) $< -MF $@

build/%.o: src/%.c Makefile linker.ld
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

build/%.elf:
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS)

build/main.elf: $(sources:src/%.c=build/%.o)

all: build/main.elf
	@arm-none-eabi-size -A -d $<

clean:
	rm -rf build

ifeq (,$(filter clean,$(MAKECMDGOALS)))
include $(sources:src/%.c=build/%.d)
endif