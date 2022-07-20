#ifndef GS_GS_H
#define GS_GS_H

#include "gs/callback.h"
#include "gs/ctx.h"
#include "gs/export.h"
#include "gs/rc.h"
#include "gs/task.h"
#include <stdbool.h>

GS_API bool gs_start(void);
GS_API void gs_stop(void);
GS_API bool gs_work(void);
GS_API void gs_sleep(double seconds);

#endif
