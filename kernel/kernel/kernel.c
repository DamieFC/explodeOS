#include <stdio.h>
#include <kernel/log/log.h>
#include <kernel/tty.h>
 
void kernel_main(void) {
    terminal_initialize();
    log(INFO, "Terminal initialized");
    printf("Welcome to explodeOS!\n");
    printf("This is from OSDev.org with modifications from DamieFC on Github!\n");
    printf("If you want to help, go to https://github.com/DamieFC/explodeOS and check it out!\n");
    printf("Hopefully you didn't install this on your computer, for two reasons: 1, you can't do anything, and 2, you can go to a browser window and check out the code, and contribute so it\'s actually useful!\n");
};
