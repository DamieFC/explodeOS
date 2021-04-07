#include <stdio.h>
#include <kernel/log.h>
#include <kernel/tty.h>
 
void kernel_main(void) {
    terminal_initialize();
    log(INFO, "Terminal initialized");
    printf("Welcome to explodeOS!\n");
    printf("This is from OSDev.org with modifications from DamieFC on Github!\n");
    printf("If you want to help, go to https://github.com/DamieFC/explodeOS and check it out!\n");
    printf("Hopefully you didn't install this on your computer, and you can just go to a browser window and check out the code!\n");
};
