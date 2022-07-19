#ifndef GS_GS_H
#define GS_GS_H

#include "gs/callback.h"
#include "gs/ctx.h"
#include "gs/export.h"
#include "gs/rc.h"
#include "gs/task.h"

GS_API void gs_init(void);
GS_API enum gs_rc gs_start(void);
GS_API void gs_stop(void);
GS_API void gs_sleep(double seconds);

#endif
