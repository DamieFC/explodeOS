#!/bin/sh
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp sysroot/boot/explodeos.kernel isodir/boot/explodeos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "explodeos" {
	multiboot /boot/explodeos.kernel
}
EOF
grub-mkrescue -o explodeos.iso isodir
