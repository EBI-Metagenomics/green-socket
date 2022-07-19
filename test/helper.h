#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR                                                                  \
    do                                                                         \
    {                                                                          \
        fprintf(stderr, "\nFailure at %s:%d\n", __func__, __LINE__);           \
        exit(1);                                                               \
    } while (1);

#endif
