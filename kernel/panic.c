#include <stdio.h>
#include <stdlib.h>
#include <kernel/log/log.h>
 
__attribute__((__noreturn__))
void panic(void) {
#if defined(__is_libk)
	/* TODO: Add proper kernel panic. */
	printf("kernel: panic: panic():\n");
	printf("PANIC!!!!!!!!\n\a\a\a"); /* Hopefully it beeps */
	printf("The panic happened with __is_libk defined.");
	log(PANIC, "Kernel panic happened! Yikes! Happened with __is_libk defined.");
#else
	/* TODO: Abnormally terminate the process as if by SIGABRT. */
	printf("PANIC!!!!!!!!\n\a\a\a"); /* Hopefully it beeps. */
	printf("The panic happened without __is_libk defined.");
	log(PANIC, "Kernel panic happened! Yikes! Happened without __is_libk defined.");
#endif
	while (1) { }
	__builtin_unreachable();
	printf("\a")
}
