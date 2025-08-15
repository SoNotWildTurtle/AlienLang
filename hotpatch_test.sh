#!/usr/bin/env bash
# Build kernel and ISO, then demonstrate a kernel hotpatch via gdb.
set -e
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
WORKDIR="$SCRIPT_DIR/hotpatch_build"
rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"
KERNEL_DIR="$SCRIPT_DIR/os/kernel"
if command -v x86_64-elf-gcc >/dev/null; then
    make -C "$KERNEL_DIR" >/dev/null
else
    make -C "$KERNEL_DIR" CC=gcc LD=ld >/dev/null
fi
# create iso
mkdir -p "$WORKDIR/iso/boot/grub"
cp "$KERNEL_DIR/kernel.bin" "$WORKDIR/iso/boot/"
if [ -d /usr/lib/grub/i386-pc ]; then
    cp /usr/lib/grub/i386-pc/*.mod "$WORKDIR/iso/boot/grub/"
fi
cat > "$WORKDIR/iso/boot/grub/grub.cfg" <<'CFG'
set timeout=0
menuentry "AlienOS" {
    multiboot2 /boot/kernel.bin
    boot
}
CFG
if command -v grub-mkrescue >/dev/null; then
    grub-mkrescue -o "$WORKDIR/alienos.iso" "$WORKDIR/iso" >/dev/null 2>&1
else
    echo "grub-mkrescue not found" >&2
    exit 1
fi
ADDR=$(nm "$KERNEL_DIR/kernel.bin" | awk '/ hotpatch_flag/{print "0x"$1}')
# run qemu paused and patch via gdb
qemu-system-x86_64 -cdrom "$WORKDIR/alienos.iso" -serial stdio -s -S -nographic >"$WORKDIR/qemu.log" &
QPID=$!
sleep 1
gdb -ex "target remote localhost:1234" \
    -ex "set *(int*)$ADDR = 1" \
    -ex continue -ex quit >/dev/null 2>&1
sleep 2
kill $QPID
cat "$WORKDIR/qemu.log"
