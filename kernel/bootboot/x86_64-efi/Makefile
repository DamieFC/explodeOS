ARCH               = $(shell uname -m | sed s,i[3456789]86,ia32,)
GNUEFI_INCLUDES    = -I/usr/include -I. -I/usr/include/efi -I/usr/include/efi/$(ARCH) -I/usr/include/efi/protocol
GNUEFI_CRT_OBJS    = crt0-efi-$(ARCH).o
GNUEFI_LDS         = elf_$(ARCH)_efi.lds

CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
CFLAGS = -mno-red-zone -mno-mmx -mno-sse -O2 -fpic -pedantic -Wall -Wextra -Werror -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -DCONFIG_$(ARCH) -DGNU_EFI_USE_MS_ABI --std=c11

LDFLAGS = -nostdlib
LDFLAGS += -shared -Bsymbolic -L. $(GNUEFI_CRT_OBJS)

TARGET  = bootboot.efi

all: tinflate.o smp.o $(TARGET)

%.efi: %.so
	@echo "  src		x86_64-efi (UEFI)"
	@$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $^ $@
	@printf "BOOTBOOT Loader do not " | dd conv=notrunc of=$(TARGET) bs=1 seek=78 1>/dev/null 2>/dev/null
	@$(CC) $(GNUEFI_INCLUDES) -Wall -fshort-wchar efirom.c -o efirom $(LIBS)
	@./efirom $(TARGET) ../dist/bootboot.rom || true
	@mv $(TARGET) ../dist/$(TARGET)
	@rm tinflate.o smp.o efirom

%.so: %.o
	@$(LD) $(LDFLAGS) tinflate.o smp.o $^ -o $@ -lefi -lgnuefi -T $(GNUEFI_LDS)

%.o: %.c
	@$(CC) $(GNUEFI_INCLUDES) $(CFLAGS) -c $< -o $@

%.o: %.S
	@$(CC) $(GNUEFI_INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	@rm bootboot.o $(TARGET) ../dist/$(TARGET) ../dist/bootboot.rom *.so *.efi efirom tinflate.o smp.o 2>/dev/null || true

