#ifndef GS_GS_H
#define GS_GS_H

#include "gs/export.h"
#include <stdbool.h>
#include <stddef.h>

struct gs_ctx;
struct gs_task;

GS_API void gs_init(void);
GS_API void gs_run(void);
GS_API void gs_stop(void);

#endif
