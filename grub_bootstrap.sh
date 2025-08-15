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

if ! command -v grub-mkrescue >/dev/null; then
    echo "grub-mkrescue not found; attempting installation..."
    if apt-get update && apt-get install -y --no-install-recommends grub-pc-bin xorriso >/dev/null 2>&1; then
        echo "grub-mkrescue installed"
    else
        echo "grub-mkrescue still unavailable; kernel built at os/kernel/kernel.bin"
        exit 0
    fi
fi

grub-mkrescue -o alienos.iso iso
if command -v qemu-system-x86_64 >/dev/null; then
    timeout 5 qemu-system-x86_64 -cdrom alienos.iso -boot d -nographic
else
    echo "qemu-system-x86_64 not found; ISO created at alienos.iso"
fi
