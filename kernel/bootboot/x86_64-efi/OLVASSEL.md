BOOTBOOT UEFI Implementáció
===========================

Általános leírásért lásd a [BOOTBOOT Protokoll](https://gitlab.com/bztsrc/bootboot)t.

Az [UEFI gépek](https://www.uefi.org/)en egy szabványos OS Loader alkalmazás használatos PCI Opció ROM-ként is.

Gép állapot
-----------

IRQ-k letiltva, GDT nincs meghatározva, de érvényes, IDT nincs beállítva. SSE, SMP engedélyezve. Kód felügyeleti módban, 0-ás gyűrűn
fut minden processzormagon.

Fájl rendszer meghajtók
-----------------------

Az UEFI verzióban a boot pratíción bármilyen fájl rendszer lehet, amit az EFI Simple File System Protocol támogat.
Ez az implementáció támogatja mind az SHA-XOR-CBC, mind az AES-256-CBC titkosítást.

Telepítés
---------

1. *UEFI lemez*: másold be a __bootboot.efi__-t az **_FS0:\EFI\BOOT\BOOTX64.EFI_**-be.

2. *UEFI ROM*: égesd ki a __bootboot.rom__-t, ami egy szabványos **_PCI Option ROM kép_**.

3. *GRUB*, *UEFI Boot Menedzser*: add hozzá a __bootboot.efi__-t az indítási opciókhoz.

Az EFI Shell-ből interaktívan is futtathatod a betöltőt paramétereket megadva a parancssorban.

```
FS0:\> EFI\BOOT\BOOTX64.EFI /?
BOOTBOOT LOADER (build Oct 11 2017)

SYNOPSIS
  BOOTBOOT.EFI [ -h | -? | /h | /? ] [ INITRDFILE [ ENVIRONMENTFILE [...] ] ]

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

Limitációk
----------

 - Az első 16G-nyi RAM-ot képezi le.
 - A PCI Option ROM-ot alá kell digitálisan írni ahhoz, hogy használni lehessen.
 - A tömörített initrd ROM 16M-nyi lehet.

Secure Boot
-----------

Először is, ez nem biztonságos egyáltalán. Az elnevezés egy átverés az M$ Marketing osztály részéről, hogy
magukhoz láncolják a gépeket, hogy azok csakis Windózt indítsanak. Ha teheted, kapcsold ki, egyébként sem ér
semmit, a rootkitek meg tudják kerülni az FBI által megkövetelt, de kiszivárgott Secure Boot Golden Key-el.

Ha mégis ragaszkodsz hozzá, akkor ahhoz, hogy működjön, olyan betöltő kell, amit a Microsoft digitálisan aláírt. Ezt nem
könnyű megszerezni egy egyedi betöltő számára, mert az M$ nem fogja megadni, akkor se, ha fizetsz érte. Szóval ehelyett,

1. töltsd le a [shim](https://apps.fedoraproject.org/packages/shim)-et
2. csomagold ki az SHIMX64.EFI és MMX64.EFI fájlokat az EFI\BOOT alá (ezeket aláírta az M$).
3. nevezd át az `EFI\BOOT\SHIMX64.EFI` fájlt `EFI\BOOT\BOOTX64.EFI`-re.
4. csinálj privát-publikus kulcspárt és egy x509 certet `openssl`-el.
```
openssl req -newkey rsa:4096 -nodes -keyout MOK.key -new -x509 -days 3650 -subj "/CN=BOOTBOOT/" -out MOK.crt
openssl x509 -outform DER -in MOK.crt -out MOK.cer
```
5. írd alá a __bootboot.efi__-t SHA-256 hashel az `sbsign`-t használva.
```
sbsign --key MOK.key --cert MOK.crt --out EFI/BOOT/GRUBX64.EFI bootboot.efi
```
6. másold át a MOK.cer fájlt az ESP partícióra.
7. bootolj UEFI-be, shim el fogja indítani a MOK Menedzsert. Ott válaszd ki az "Enroll key from disk" opciót, keresd meg a MOK.cer-t, add hozzá. Aztán "Enroll hash from disk", keresd meg a GRUBX64.EFI-t és add hozzá.
8. engedélyezd a Secure Boot-ot.

Ezek után a lépések után a BOOTBOOT betöltő Secure Boot módban fog indulni (shim a továbbiakban az aláírt
GRUBX64.EFI-t indítja a MOK Menedzser helyett).
