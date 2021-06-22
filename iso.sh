#!/bin/sh
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp sysroot/boot/explodeos.kernel isodir/boot/explodeos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=60
set default=0
set menu_color_normal=blue/black
set menu_color_highlight=white/blue

menuentry "explodeOS" {
	multiboot /boot/explodeos.kernel
}
menuentry "Reboot" {
    reboot
}
menuentry "Shut Down" {
	halt
}
EOF
grub-mkrescue -o explodeos.iso isodir
