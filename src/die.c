#include "die.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void die(char const *msg)
{
    fputs(msg, stderr);
    exit(1);
}
