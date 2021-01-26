#
# Makefile for clang compiler for embedded aarch64 RaspberryPi3 code
# Created at 25/01/2021
# Written by m3m0ry
#

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CC = /usr/local/opt/llvm/bin/clang
LD = /usr/local/opt/llvm/bin/ld.lld
OBJCOPY = /usr/local/opt/llvm/bin/llvm-objcopy
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

all: clean kernel8.img

start.o: start.S
	clang --target=aarch64-elf $(CFLAGS) -c start.S -o start.o

%.o: %.c
	clang --target=aarch64-elf $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	$(LD) -m aarch64elf -nostdlib start.o $(OBJS) -T link.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio


