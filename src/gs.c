#include "gs/gs.h"
#include "ctx.h"
#include "ev/ev.h"
#include "task.h"

struct ev_loop *loop = 0;

void gs_init(void) { loop = EV_DEFAULT; }

void gs_run(void) { ev_run(loop, 0); }

void gs_stop(void) { ev_break(loop, EVBREAK_ALL); }

struct gs_task *gs_send(struct gs_ctx *ctx, void *data, gs_write_cb write_cb,
                        gs_when_done_cb when_done_cb, double timeout)
{
    struct gs_task *task = gs_ctx_pop(ctx);
    if (!task) return 0;
    gs_task_init(task, timeout);
    task->data = data;
    task->write_cb = write_cb;
    task->when_done_cb = when_done_cb;
    gs_task_start(task);
    return task;
}
