BOOTBOOT UEFI Implementation
============================

See [BOOTBOOT Protocol](https://gitlab.com/bztsrc/bootboot) for common details.

On [UEFI machines](http://www.uefi.org/), the PCI Option ROM is created from the standard EFI
OS loader application.

Machine state
-------------

IRQs masked. GDT unspecified, but valid, IDT unset. SSE, SMP enabled. Code is running in supervisor mode in ring 0 on all cores.

File system drivers
-------------------

For boot partition, UEFI version relies on any file system that's supported by EFI Simple File System Protocol.
This implementation supports both SHA-XOR-CBC and AES-256-CBC cipher.

Installation
------------

1. *UEFI disk*: copy __bootboot.efi__ to **_FS0:\EFI\BOOT\BOOTX64.EFI_**.

2. *UEFI ROM*: use __bootboot.rom__ which is a standard **_PCI Option ROM image_**.

3. *GRUB*, *UEFI Boot Manager*: add __bootboot.efi__ to boot options.

You can also run the loader in interactive mode from the EFI Shell, appending options to its command line.

```
FS0:\> EFI\BOOT\BOOTX64.EFI /?
BOOTBOOT LOADER (build Oct 11 2017)

SYNOPSIS
  BOOTBOOT.EFI [ -h | -? | /h | /? | -s ] [ INITRDFILE [ ENVIRONFILE [...] ] ]

DESCRIPTION
  Bootstraps an operating system via the BOOTBOOT Protocol.
  If arguments not given, defaults to
    FS0:\BOOTBOOT\INITRD   as ramdisk image and
    FS0:\BOOTBOOT\CONFIG   for boot environment.
  Additional "key=value" command line arguments will be appended to the
  environment. If INITRD not found, it will use the first bootable partition
  in GPT. If CONFIG not found, it will look for /sys/config inside the
  INITRD (or partition). With -s it will scan the memory for an initrd ROM.

  As this is a loader, it is not supposed to return control to the shell.

FS0:\>
```

Limitations
-----------

 - Maps the first 16G of RAM.
 - PCI Option ROM should be signed in order to work.
 - Compressed initrd in ROM is limited to 16M.

Secure Boot
-----------

First of all, this does not mean Secure. The naming is just a fraud from the M$ Marketing department to
lock-in machines to only boot Windoze. If you can, turn it off, it worths nothing anyway as rootkits can
workaround it using the leaked Secure Boot Golden Key backdoor demanded by the FBI.

If despite that you insist, then to get it to work, you'll need a loader that is signed by Microsoft. It is
not easy to get your custom loader signed, because M$ just won't do that even if you pay for it. So instead,

1. download [shim](https://apps.fedoraproject.org/packages/shim)
2. extract SHIMX64.EFI and MMX64.EFI to EFI\BOOT (these are signed by M$).
3. rename `EFI\BOOT\SHIMX64.EFI` to `EFI\BOOT\BOOTX64.EFI`.
4. create a public-private key pair and x509 cert with `openssl`.
```
openssl req -newkey rsa:4096 -nodes -keyout MOK.key -new -x509 -days 3650 -subj "/CN=BOOTBOOT/" -out MOK.crt
openssl x509 -outform DER -in MOK.crt -out MOK.cer
```
5. sign __bootboot.efi__ using an SHA-256 hash with `sbsign`.
```
sbsign --key MOK.key --cert MOK.crt --out EFI/BOOT/GRUBX64.EFI bootboot.efi
```
6. copy MOK.cer to the ESP partition.
7. boot into UEFI, shim will call MOK Manager. There select "Enroll key from disk", find MOK.cer and add it. Then select "Enroll hash from disk", find GRUBX64.EFI and add it.
8. enable Secure Boot.

After these steps BOOTBOOT loader will boot with Secure Boot enabled (shim will load the signed GRUBX64.EFI
instead of the MOK Manager hereafter).
