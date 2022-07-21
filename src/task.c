#include "task.h"
#include "ctx.h"
#include "debug.h"
#include "die.h"
#include "gs/rc.h"
#include "gs/task.h"
#include "loop.h"
#include "socket_include.h"
#include <errno.h>
#include <stdlib.h>

static enum gs_rc read_fn(struct gs_task *task, size_t size, void *buf,
                          size_t *size_read)
{
    task->errno_value = 0;

    ssize_t sz = read(gs_ctx_socket(task->ctx), buf, size);
    if (sz == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            *size_read = 0;
            return GS_EOF;
        }
        task->errno_value = errno;
        return GS_ERROR;
    }
    *size_read = sz;
    return GS_OK;
}

static enum gs_rc write_fn(struct gs_task *task, size_t size, void const *buf,
                           size_t *size_written)
{
    task->errno_value = 0;

    ssize_t sz = write(gs_ctx_socket(task->ctx), buf, size);
    if (sz == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            *size_written = 0;
            return GS_EOF;
        }
        task->errno_value = errno;
        return GS_ERROR;
    }
    *size_written = sz;
    return GS_OK;
}

static void read_cb(struct ev_io *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event during read");
    struct gs_task *task = w->data;

    if (!(*task->read_cb)(task, read_fn))
    {
        debug();
        gs_loop_timer_again_within(&task->watcher.timeout);
    }
    else
    {
        debug();
        gs_task_cancel(task);
    }
}

static void write_cb(ev_io *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event during write");
    struct gs_task *task = w->data;

    if (!(*task->write_cb)(task, write_fn))
    {
        debug();
        gs_loop_timer_again_within(&task->watcher.timeout);
    }
    else
    {
        debug();
        gs_task_cancel(task);
    }
}

static void timeout_cb(ev_timer *w, int revents)
{
    if (EV_ERROR & revents) die("invalid event during timeout");
    struct gs_task *task = w->data;

    debug();
    gs_task_cancel(task);
}

void gs_task_init(struct gs_task *task, struct gs_ctx *ctx)
{
    task->data = 0;

    task->ctx = ctx;

    task->active = false;
    task->done = false;
    debug();
    atomic_flag_clear(&task->cancel_once);
    task->cancelled = false;
    task->errno_value = 0;

    task->type = GS_TASK_NOTSET;

    task->read_cb = 0;
    task->write_cb = 0;
    task->when_done_cb = 0;

    gs_loop_init(&task->watcher.read, read_cb);
    gs_loop_init(&task->watcher.write, write_cb);
    gs_loop_init(&task->watcher.timeout, timeout_cb);

    task->watcher.read.data = task;
    task->watcher.write.data = task;
    task->watcher.timeout.data = task;

    task->timeout = 0.;
    cco_node_init(&task->node);
}

void gs_task_reset(struct gs_task *task, double timeout)
{
    task->data = 0;

    task->active = false;
    task->done = false;
    debug();
    atomic_flag_clear(&task->cancel_once);
    task->cancelled = false;
    task->errno_value = 0;

    task->type = GS_TASK_NOTSET;

    task->read_cb = 0;
    task->write_cb = 0;
    task->when_done_cb = 0;

    task->timeout = timeout;
    cco_node_init(&task->node);
}

void gs_task_setup_send(struct gs_task *task, void *data, gs_write_cb *write_cb,
                        gs_when_done_cb *when_done_cb)
{
    task->data = data;
    task->type = GS_TASK_SEND;
    task->write_cb = write_cb;
    task->when_done_cb = when_done_cb;
}

void gs_task_setup_recv(struct gs_task *task, void *data, gs_read_cb *read_cb,
                        gs_when_done_cb *when_done_cb)
{
    task->data = data;
    task->type = GS_TASK_RECV;
    task->read_cb = read_cb;
    task->when_done_cb = when_done_cb;
}

void gs_task_start(struct gs_task *task)
{
    task->active = true;
    task->watcher.timeout.repeat = task->timeout;
    gs_loop_timer_again(&task->watcher.timeout);
    if (task->type == GS_TASK_SEND)
    {
        gs_loop_io_set(&task->watcher.write, gs_ctx_socket(task->ctx),
                       EV_WRITE);
        gs_loop_io_start(&task->watcher.write);
    }
    if (task->type == GS_TASK_RECV)
    {
        gs_loop_io_set(&task->watcher.read, gs_ctx_socket(task->ctx), EV_READ);
        gs_loop_io_start(&task->watcher.read);
    }
}

bool gs_task_done(struct gs_task const *task) { return task->done; }

void gs_task_cancel(struct gs_task *task)
{
    debug();
    if (atomic_flag_test_and_set(&task->cancel_once)) return;
    if (!task->active) return;
    debug();
    task->cancelled = true;

    if (task->type == GS_TASK_SEND)
        gs_loop_io_stop_within(&task->watcher.write);
    if (task->type == GS_TASK_RECV) gs_loop_io_stop_within(&task->watcher.read);
    gs_loop_timer_stop_within(&task->watcher.timeout);
    task->done = true;
    (*task->when_done_cb)(task);
    task->active = false;
}

bool gs_task_cancelled(struct gs_task const *task) { return task->cancelled; }

int gs_task_errno(struct gs_task const *task) { return task->errno_value; }

void gs_task_del(struct gs_task const *task)
{
    gs_ctx_put(task->ctx, (struct gs_task *)task);
}
