#ifndef GS_CTX_H
#define GS_CTX_H

#include "gs/callback.h"
#include "gs/export.h"

struct gs_ctx;
struct gs_task;

GS_API struct gs_ctx *gs_ctx_new(int sockfd, unsigned max_tasks);
GS_API void gs_ctx_del(struct gs_ctx const *);

GS_API struct gs_task *gs_ctx_send(struct gs_ctx *ctx, void *data, gs_write_cb,
                                   gs_when_done_cb, double timeout);

#endif
