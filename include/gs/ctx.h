#ifndef GS_CTX_H
#define GS_CTX_H

#include "gs/export.h"

struct gs_ctx;
struct gs_task;

GS_API struct gs_ctx *gs_ctx_new(int sockfd, unsigned max_tasks);
GS_API void gs_ctx_del_task(struct gs_ctx *, struct gs_task const *);
GS_API void gs_ctx_del(struct gs_ctx const *);

#endif
