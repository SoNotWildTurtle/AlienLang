CC?=x86_64-elf-gcc
LD?=x86_64-elf-ld
AS=nasm
# Build as a freestanding 64-bit binary. Disable the red zone so interrupts
# can safely use the same stack and use large model code generation so the
# linker script can place the kernel at a high address.
CFLAGS=-m64 -ffreestanding -mno-red-zone -mcmodel=large -O2 -Wall -Wextra -fno-pie
LDFLAGS=-m elf_x86_64 -T link.ld
OBJCOPY=objcopy

all: kernel.bin

boot.o: boot.s
	$(AS) -f elf64 $< -o $@

isr_stub.o: isr_stub.s
	$(AS) -f elf64 $< -o $@

kernel.o: kernel.c mem.h wah.h alienvm.h net.h fs.h pkg.h timer.h isr.h
	$(CC) $(CFLAGS) -c $< -o $@

sched.o: sched.c sched.h mem.h wah.h
	$(CC) $(CFLAGS) -c sched.c -o sched.o

mem.o: mem.c mem.h
	$(CC) $(CFLAGS) -c mem.c -o mem.o

paging.o: paging.c paging.h mem.h
	$(CC) $(CFLAGS) -c paging.c -o paging.o

pkg.o: pkg.c pkg.h fs.h mem.h
	$(CC) $(CFLAGS) -c pkg.c -o pkg.o

wah.o: wah.c wah.h sched.h wah_msg.h cfg.h
	$(CC) $(CFLAGS) -c wah.c -o wah.o

rtc.o: rtc.c rtc.h io.h
	$(CC) $(CFLAGS) -c rtc.c -o rtc.o

wah_msg.o: wah_msg.c wah_msg.h net.h
	$(CC) $(CFLAGS) -c wah_msg.c -o wah_msg.o

wah_vm.o: ../../wah/wah_vm.c ../../wah/wah_vm.h log.h
	$(CC) $(CFLAGS) -I . -I ../../os/kernel -c ../../wah/wah_vm.c -o wah_vm.o

reboot.o: reboot.c reboot.h io.h
	$(CC) $(CFLAGS) -c reboot.c -o reboot.o

security.o: security.c security.h sched.h
	$(CC) $(CFLAGS) -c security.c -o security.o

log.o: log.c log.h
	$(CC) $(CFLAGS) -c log.c -o log.o

cfg.o: cfg.c cfg.h
	$(CC) $(CFLAGS) -c cfg.c -o cfg.o

fs.o: fs.c fs.h mem.h
	$(CC) $(CFLAGS) -c fs.c -o fs.o

alienvm.o: alienvm.c alienvm.h wah.h sched.h
	$(CC) $(CFLAGS) -c alienvm.c -o alienvm.o

jit.o: jit.c alienvm.h
	$(CC) $(CFLAGS) -c jit.c -o jit.o

blockchain.o: blockchain.c blockchain.h wah.h sched.h
	$(CC) $(CFLAGS) -c blockchain.c -o blockchain.o

net.o: net.c net.h mem.h
	$(CC) $(CFLAGS) -c net.c -o net.o

random.o: random.c random.h
	$(CC) $(CFLAGS) -c random.c -o random.o

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c -o hash.o

device.o: device.c device.h
	$(CC) $(CFLAGS) -c device.c -o device.o

timer.o: timer.c timer.h io.h isr.h sched.h
	$(CC) $(CFLAGS) -c timer.c -o timer.o

isr.o: isr.c isr.h
	$(CC) $(CFLAGS) -c isr.c -o isr.o



kernel.bin: boot.o isr_stub.o kernel.o mem.o paging.o pkg.o sched.o wah.o wah_msg.o wah_vm.o fs.o alienvm.o jit.o blockchain.o net.o random.o hash.o device.o timer.o isr.o security.o log.o cfg.o rtc.o reboot.o
	$(CC) -T link.ld boot.o isr_stub.o kernel.o mem.o paging.o pkg.o sched.o wah.o wah_msg.o wah_vm.o fs.o alienvm.o jit.o blockchain.o net.o random.o hash.o device.o timer.o isr.o security.o log.o cfg.o rtc.o reboot.o -o kernel.elf -nostdlib -lgcc -no-pie
	$(OBJCOPY) -O binary kernel.elf $@

clean:
	rm -f *.o kernel.elf kernel.bin
