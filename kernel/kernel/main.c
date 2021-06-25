#include <kernel/print.h>
// #include <kernel/log/log.h>
#include <kernel/tty.h>
#include <../arch/i386/vga.h>

void kernel_main(void) {
    terminal_initialize();
    terminal_setcolor(VGA_COLOR_BROWN);
    // log(INFO, "Terminal initialized");
    printf("                 _           _       _____ _____ \n");
    printf("                | |         | |     |  _  /  ___|\n");
    printf("  _____  ___ __ | | ___   __| | ___ | | | \\ `--. \n");
    printf(" / _ \\ \\/ / '_ \\| |/ _ \\ / _` |/ _ \\| | | |`--. \n");
    printf("|  __/>  <| |_) | | (_) | (_| |  __/\\ \\_/ /\\__/ /\n");
    printf(" \\___/_/\\_\\ .__/|_|\\___/ \\__,_|\\___| \\___/\\____/\n");
    printf("          | |                                    \n");
    printf("          |_|                                    \n");

    terminal_setcolor(VGA_COLOR_WHITE);

    printf("Welcome to explodeOS!\n");
    printf("This is from OSDev.org with modifications from DamieFC on Github!\n");
    printf("If you want to help, go to https://github.com/DamieFC/explodeOS and check it out!\n");
    printf("Hopefully you didn't install this on your computer, for two reasons: 1, you can't do anything, and 2, you can go to a browser window and check out the code, and contribute so it\'s actually useful!\n");
};
