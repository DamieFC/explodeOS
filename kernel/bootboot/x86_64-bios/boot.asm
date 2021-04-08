;*
;* x86_64-bios/boot.asm
;*
;* Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
;*
;* Permission is hereby granted, free of charge, to any person
;* obtaining a copy of this software and associated documentation
;* files (the "Software"), to deal in the Software without
;* restriction, including without limitation the rights to use, copy,
;* modify, merge, publish, distribute, sublicense, and/or sell copies
;* of the Software, and to permit persons to whom the Software is
;* furnished to do so, subject to the following conditions:
;*
;* The above copyright notice and this permission notice shall be
;* included in all copies or substantial portions of the Software.
;*
;* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;* DEALINGS IN THE SOFTWARE.
;*
;* This file is part of the BOOTBOOT Protocol package.
;* @brief Stage1 loader, compatible with BIOS boot specification and
;* El Torito CD-ROM boot in "no emulation" mode
;*

;*********************************************************************
;*                             Macros                                *
;*********************************************************************

;LBA packet fields
lba_packet      equ     07E00h
virtual at lba_packet
lbapacket.size: dw      ?
lbapacket.count:dw      ?
lbapacket.addr0:dw      ?
lbapacket.addr1:dw      ?
lbapacket.sect0:dw      ?
lbapacket.sect1:dw      ?
lbapacket.sect2:dw      ?
lbapacket.sect3:dw      ?
end virtual

;memory locations
ldr.header      equ     800h        ;position of 2nd stage loader
ldr.executor    equ     804h        ;ptr to init code

;Writes a message on screen.
macro print msg
{
if ~ msg eq si
            push        si
            mov         si, msg
end if
            call        printfunc
if ~ msg eq si
            pop         si
end if
}

;*********************************************************************
;*                             code                                  *
;*********************************************************************

;-----------------ENTRY POINT called by BIOS--------------------
            ORG         0600h
            USE16

bootboot_record:
            jmp         short .skipid
            nop
            ;skip BPB area so that we can use this
            ;boot code on a FAT / exFAT volume if needed
            db          120-($-$$) dup 0
.skipid:    ;relocate our code to offset 0h:600h
            cli
            cld
            xor         ax, ax
            mov         ss, ax
            mov         sp, 600h
            push        ax
            pop         es
            push        cs
            pop         ds
            ;find our position in memory.
            call        .getaddr
.getaddr:   pop         si
            sub         si, .getaddr-bootboot_record
            mov         di, sp
            ;clear data area 500h-600h
            sub         di, 100h
            mov         cx, 80h
            repnz       stosw
            ;and copy ourselves to 600h
            mov         cx, 100h
            repnz       movsw
            ;have to clear ds, because cs is set to 7c0 when booted from El Torito
            push        es
            pop         ds
            jmp         0:.start

.start:     ;save boot drive code
            mov         byte [drive], dl
            ;check for lba presistance - floppy not supported any more
            ;we use USB sticks as removable media for a long time
            cmp         dl, byte 80h
            jl          .nolba
.notfloppy: mov         ah, byte 41h
            mov         bx, word 55AAh
            int         13h
            jc          .nolba
            cmp         bx, word 0AA55h
            jne         .nolba
            test        cl, byte 1
            jnz         .lbaok
.nolba:     mov         si, lbanotf
            jmp         diefunc
.lbaok:     ;get CDROM drive code
            mov         ax, word 4B01h
            mov         si, lba_packet
            mov         byte [si + 2], 0E0h
            push        si
            int         13h
            pop         si
            jc          .read2stg
            mov         al, byte [si + 2]
            mov         byte [cdrom], al
            ;try to load stage2 - it's a continous area on disk
            ;started at given sector with maximum size of 7000h bytes
.read2stg:  mov         di, lba_packet
            mov         si, stage2_addr
            ;set up for hard-drive
            xor         ah, ah
            mov         al, 16          ; size
            stosw
            mov         al, 56          ; count
            stosw
            xor         al, al
            mov         ah, 8           ; addr0 to address 800h
            stosw
            xor         ax, ax          ; addr1
            stosw
            movsw                       ; sect0
            movsw                       ; sect1
            xor         ax, ax
            stosw                       ; sect2
            stosw                       ; sect3
            mov         dl, byte [drive]
            ;if it's a CDROM with 2048 byte sectors
            cmp         dl, byte [cdrom]
            jl          @f
            sub         di, 8
            ;lba=lba/4
            clc
            rcr         word [di+2], 1
            rcr         word [di], 1
            clc
            rcr         word [di+2], 1
            rcr         word [di], 1
            ;count=count/4
            shr         word [lbapacket.count], 2
            ;load sectors
@@:         mov         ah, byte 42h
            mov         si, lba_packet
            int         13h

            ;do we have a 2nd stage loader?
.chk:       cmp         word [ldr.header], 0AA55h
            jne         .nostage2
            cmp         byte [ldr.header+3], 0E9h
            jne         .nostage2
            mov         dl, byte [drive]
            ;invoke stage2 real mode code
            mov         ax, [ldr.executor]
            add         ax, ldr.executor+2
            jmp         ax

.nostage2:  ;try to load stage2 from a RAID mirror
            mov         al, byte [drive]
            inc         al
            cmp         al, 87h
            jle         @f
            mov         al, 80h
@@:         mov         byte [drive], al
            inc         byte [cnt]
            cmp         byte [cnt], 8
            jl          .read2stg
.nostage2err:
            mov         si, stage2notf
            ;fall into the diefunc code

;*********************************************************************
;*                          functions                                *
;*********************************************************************
;writes the reason, waits for a key and reboots.
diefunc:
            print       panic
            call        printfunc
            sti
            xor         ax, ax
            int         16h
            mov         al, 0FEh
            out         64h, al
            jmp         far 0FFFFh:0    ;invoke BIOS POST routine

;ds:si zero terminated string to write
printfunc:
            lodsb
            or          al, al
            jz          .end
            mov         ah, byte 0Eh
            mov         bx, word 11
            int         10h
            jmp         printfunc
.end:       ret

;*********************************************************************
;*                          data area                                *
;*********************************************************************

panic:      db          "BOOTBOOT-PANIC: no ",0
lbanotf:    db          "LBA support",0
stage2notf: db          "FS0:\BOOTBOOT.BIN",0
            db          01B0h-($-$$) dup 0

;right before the partition table some data
stage2_addr:dd          0FFFFFFFFh      ;1B0h 2nd stage loader address
                                        ;this should be set by mkfs
cnt:        db          0
drive:      db          0
cdrom:      db          0
            db          0

diskid:     dd          0               ;1B8h WinNT expects it here
            dw          0

;1BEh first partition entry

            ;padding and magic
            db          01FEh-($-$$) dup 0
            db          55h,0AAh
bootboot_record_end:
