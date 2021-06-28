#include <stdio.h>
#include <kernel/log/log.h>
void panic(const char *pm) {
    printf("Kernel panic: %s", pm);
    log(PANIC, "Kernel panic: %s", pm);
    asm("hlt");
}
