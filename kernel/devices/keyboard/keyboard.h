/**************************************************************
 * explodeOS keykeyboard things for keyboard                  *
 * (c) DamieFC 2021 under the MIT license                     *
 **************************************************************/
#include <stdio.h>

char keymap[6 /* So far */] = {
    /* Help me pls I don't really get bytes */
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E /* OK literally esc 1 2 3 4 5 6 7 8 9 0 - = backspace */
}

void keyboard_init();
