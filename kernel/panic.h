#ifndef PANIC_H
#define PANIC_H
#include <kernel/log/log.h>
void panic(void){
    log(PANIC, "PANIC!");
    printf("PANIC!!!!!!!\n\a\a\a\a\a\a\a");
}
