#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define debug()                                                                \
    do                                                                         \
    {                                                                          \
        printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);                      \
        fflush(stdout);                                                        \
    } while (0);

#endif
