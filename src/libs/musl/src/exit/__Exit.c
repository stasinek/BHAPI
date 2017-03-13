#include <stdlib.h>
#include "syscall.h"

_Noreturn void __Exit(int ec)
{
	__syscall(SYS_exit_group, ec);
	for (;;) __syscall(SYS_exit, ec);
}
