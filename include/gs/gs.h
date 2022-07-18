#ifndef GS_GS_H
#define GS_GS_H

#include "gs/callback.h"
#include "gs/export.h"
#include <stdbool.h>
#include <stddef.h>

struct gs_ctx;
struct gs_task;

GS_API void gs_init(void);
GS_API void gs_run(void);
GS_API void gs_stop(void);
GS_API struct gs_task *gs_send(struct gs_ctx *ctx, void *data, gs_write_cb,
                               gs_when_done_cb, double timeout);

#endif
