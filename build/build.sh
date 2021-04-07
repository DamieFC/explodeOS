## NOTE: I DIDN'T WRITE THIS. I JUST STUCK IT TOGETHER FROM THE OSDEV.ORG PAGE. (C) OSDEV UNDER THE MIT LICENSE. ##
###################################################################################################################

# Assembler
i686-elf-as boot.s -o boot.o
# C Compiler
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
# Linker
i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
# Verify multiboot
grub-file --is-x86-multiboot myos.bin
# Making sure grub-file is there
if grub-file --is-x86-multiboot myos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi
# Making a boot image
mkdir -p isodir/boot/grub
cp myos.bin isodir/boot/myos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o myos.iso isodir
# Now run in QEMU!
qemu-system-i386 -cdrom myos.iso
