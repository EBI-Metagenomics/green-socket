#ifndef CTX_H
#define CTX_H

struct gs_ctx;
struct gs_task;

int gs_ctx_socket(struct gs_ctx *);
struct gs_task *gs_ctx_pop(struct gs_ctx *);
void gs_ctx_put(struct gs_ctx *, struct gs_task *);

#endif
