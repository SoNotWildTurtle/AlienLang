#!/usr/bin/env bash
# Quickly build a minimal ISO containing the toy kernel.
set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
WORKDIR="grub_build"
mkdir -p "$WORKDIR"
cd "$WORKDIR"

# Build the tiny kernel
KERNEL_DIR="$SCRIPT_DIR/os/kernel"
if command -v x86_64-elf-gcc >/dev/null; then
    make -C "$KERNEL_DIR"
else
    make -C "$KERNEL_DIR" CC=gcc LD=ld
fi

# Use system GRUB modules to create an ISO
mkdir -p iso/boot/grub
cp "$KERNEL_DIR/kernel.bin" iso/boot/
if [ -d /usr/lib/grub/i386-pc ]; then
    cp /usr/lib/grub/i386-pc/*.mod iso/boot/grub/
fi
cat > iso/boot/grub/grub.cfg <<'CFG'
set timeout=5
set default=0

menuentry "AlienOS" {
    echo "Booting AlienOS..."
    multiboot2 /boot/kernel.bin
    boot
}
CFG

if command -v grub-mkrescue >/dev/null; then
    grub-mkrescue -o alienos.iso iso
    timeout 5 qemu-system-x86_64 -cdrom alienos.iso -boot d -nographic
else
    echo "grub-mkrescue not found; kernel built at os/kernel/kernel.bin"
fi
