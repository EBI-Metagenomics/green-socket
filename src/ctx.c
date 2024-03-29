#include "gs/ctx.h"
#include "cco/cco.h"
#include "ctx.h"
#include "gs/rc.h"
#include "gs/task.h"
#include "task.h"
#include <stdlib.h>

struct gs_ctx
{
    void *data;
    int sockfd;

    struct
    {
        struct cco_queue avail;
        struct gs_task tasks[];
    } task;
};

int gs_ctx_socket(struct gs_ctx *ctx) { return ctx->sockfd; }

struct gs_task *gs_ctx_pop(struct gs_ctx *ctx)
{
    if (cco_queue_empty(&ctx->task.avail)) return 0;
    return cco_of(cco_queue_pop(&ctx->task.avail), struct gs_task, node);
}

void gs_ctx_put(struct gs_ctx *ctx, struct gs_task *task)
{
    cco_queue_put(&ctx->task.avail, (struct cco_node *)&task->node);
}

struct gs_ctx *gs_ctx_new(int sockfd, unsigned max_tasks)
{
    struct gs_ctx *ctx =
        malloc(sizeof(struct gs_ctx) + max_tasks * sizeof(struct gs_task));

    if (!ctx) return 0;

    ctx->data = 0;
    ctx->sockfd = sockfd;

    cco_queue_init(&ctx->task.avail);
    for (unsigned i = 0; i < max_tasks; ++i)
    {
        gs_task_init(ctx->task.tasks + i, ctx);
        cco_queue_put(&ctx->task.avail, &ctx->task.tasks[i].node);
    }

    return ctx;
}

void gs_ctx_del(struct gs_ctx const *ctx)
{
    struct cco_queue *q = (struct cco_queue *)&ctx->task.avail;
    while (!cco_queue_empty(&ctx->task.avail))
    {
        struct gs_task *t = cco_of(cco_queue_pop(q), struct gs_task, node);
        gs_task_cancel(t);
    }
    free((void *)ctx);
}

struct gs_task *gs_ctx_send(struct gs_ctx *ctx, void *data,
                            gs_write_cb *write_cb,
                            gs_when_done_cb *when_done_cb, double timeout)
{
    struct gs_task *task = gs_ctx_pop(ctx);
    if (!task) return 0;
    gs_task_reset(task, timeout);
    gs_task_setup_send(task, data, write_cb, when_done_cb);
    gs_task_start(task);
    return task;
}
