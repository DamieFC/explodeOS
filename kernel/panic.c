#include <stdio.h>
#include <stdlib.h>
 
__attribute__((__noreturn__))
void panic(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	printf("kernel: panic: panic()\n");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("panic()\n");
#endif
	while (1) { }
	__builtin_unreachable();
}
