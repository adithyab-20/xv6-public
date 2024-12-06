#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    strace_on();  // Enable tracing
    printf(1, "Hello, world!\n");  // System call: write
    exit();  // System call: exit
}