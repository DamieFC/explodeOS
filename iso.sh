#!/bin/sh
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp sysroot/boot/explodeOS.kernel isodir/boot/explodeOS.kernel
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=60
set default=0
set menu_color_normal=brown/black
set menu_color_highlight=white/brown

menuentry "explodeOS" {
	multiboot /boot/explodeOS.kernel
}
menuentry "Reboot" {
    reboot
}
menuentry "Shut Down" {
	halt
}
EOF
grub-mkrescue -o explodeOS.iso isodir
