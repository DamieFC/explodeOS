all:
	@# NOTE: GNU assembler cannot mix 16 bit real mode with 32 bit prot mode instructions
	@# as easily as I need it in a boot loader. So I decided to use fasm.
	@echo "  src		x86_64-bios (MultiBoot / BIOS)"
	@fasm boot.asm ../dist/boot.bin >/dev/null
	@fasm bootboot.asm ../dist/bootboot.bin >/dev/null

mkboot: ../dist/boot.bin mkboot.c
	@echo "  src		mkboot"
	@ld -r -b binary -o boot.o ../dist/boot.bin
	@gcc boot.o mkboot.c -o mkboot
	@rm boot.o 2>/dev/null || true

clean:
	@rm *.o ../mbr.bin ../bootboot.bin mkboot >/dev/null 2>/dev/null || true
