#include "gs/gs.h"
#include "ctx.h"
#include "ev/ev.h"
#include "task.h"
// struct gs_ctx
// {
// 	void *data;
// 	int socket;
//
// 	struct gs_ctx_priv *priv;
// };
//
// struct gs_ctx *ctx;
//
// struct gs_task *task = gs_send(ctx, data, write_to, when_done, timeout);
//
// gs_task_done(task);
// gs_task_cancel(task);
// gs_task_cancelled(task);
// gs_task_error(task);
// gs_task_free(task);
//
//
// enum gs_rc gs_write_t(size_t buffer_size, void const* buffer, size_t*
// bytes_written);
//
//
// void write_to(struct gs_task* task, gs_write_t write_fn)
// {
// 	obj = task->data;
//
// 	size_t size = obj->buffer_size;
// 	unsigned char * buffer = obj->buffer;
// 	size_t bytes_written = 0;
//
// 	enum gs_rc rc = 0;
// 	while (!(rc = write_fn(size, buffer, &bytes_written)))
// 		buffer += bytes_written;
// }
//
// task = gs_recv(ctx, data, read_from, when_done, timeout);
//
static struct ev_loop *loop = 0;

void gs_init(void) { loop = EV_DEFAULT; }

void gs_run(void) { ev_run(loop, 0); }

void gs_stop(void) { ev_break(EV_A_ EVBREAK_ALL); }

struct gs_task *gs_send(struct gs_ctx *ctx, void *data, gs_write_cb write_cb,
                        gs_when_done_cb when_done_cb, double timeout)
{
    struct gs_task *task = gs_ctx_pop(ctx);
    if (!task) return 0;
    gs_task_init(task, timeout);
    task->data = data;
    task->write_cb = write_cb;
    task->when_done_cb = when_done_cb;
    return task;
}
