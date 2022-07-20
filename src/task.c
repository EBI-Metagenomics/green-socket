#include "task.h"
#include "ctx.h"
#include "die.h"
#include "gs.h"
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

static void read_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    (void)loop;
    if (EV_ERROR & revents) die("invalid event during read");
    struct gs_task *task = w->data;

    if (!(*task->read_cb)(task, read_fn))
    {
        ev_timer_again(loop, &task->watcher.timeout);
    }
    else
    {
        ev_io_stop(loop, &task->watcher.read);
        ev_timer_stop(loop, &task->watcher.timeout);
        (*task->when_done_cb)(task);
    }
}

#include <stdio.h>
static void write_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    printf("write_cb1\n");
    fflush(stdout);
    (void)loop;
    if (EV_ERROR & revents) die("invalid event during write");
    printf("write_cb2\n");
    fflush(stdout);
    struct gs_task *task = w->data;

    printf("write_cb3\n");
    fflush(stdout);
    if (!(*task->write_cb)(task, write_fn))
    {
        ev_timer_again(loop, &task->watcher.timeout);
    }
    else
    {
        ev_io_stop(loop, &task->watcher.read);
        ev_timer_stop(loop, &task->watcher.timeout);
        (*task->when_done_cb)(task);
    }
}

static void timeout_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
    (void)loop;
    if (EV_ERROR & revents) die("invalid event during timeout");
    struct gs_task *task = w->data;

    gs_task_cancel(task);
}

void gs_task_init(struct gs_task *task, struct gs_ctx *ctx)
{
    task->data = 0;

    task->ctx = ctx;

    task->done = false;
    task->cancelled = false;
    task->errno_value = 0;

    task->type = GS_TASK_NOTSET;

    task->read_cb = 0;
    task->write_cb = 0;
    task->when_done_cb = 0;

    gs_loop_ev_init(&task->watcher.read, read_cb);
    gs_loop_ev_init(&task->watcher.write, write_cb);
    gs_loop_ev_init(&task->watcher.timeout, timeout_cb);

    task->watcher.read.data = task;
    task->watcher.write.data = task;
    task->watcher.timeout.data = task;

    task->timeout = 0.;
    cco_node_init(&task->node);
}

void gs_task_reset(struct gs_task *task, double timeout)
{
    task->data = 0;

    task->done = false;
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
    task->watcher.timeout.repeat = task->timeout;
    gs_loop_ev_timer_again(&task->watcher.timeout);
    if (task->type == GS_TASK_SEND)
    {
        gs_loop_ev_io_set(&task->watcher.write, gs_ctx_socket(task->ctx),
                          EV_WRITE);
        gs_loop_ev_io_start(&task->watcher.write);
    }
    if (task->type == GS_TASK_RECV)
    {
        gs_loop_ev_io_set(&task->watcher.read, gs_ctx_socket(task->ctx),
                          EV_READ);
        gs_loop_ev_io_start(&task->watcher.read);
    }
}

bool gs_task_done(struct gs_task const *task) { return task->done; }

void gs_task_cancel(struct gs_task *task)
{
    gs_loop_ev_io_stop(&task->watcher.read);
    gs_loop_ev_io_stop(&task->watcher.write);
    gs_loop_ev_timer_stop(&task->watcher.timeout);
    task->done = true;
    task->cancelled = true;
}

bool gs_task_cancelled(struct gs_task const *task) { return task->cancelled; }

int gs_task_errno(struct gs_task const *task) { return task->errno_value; }

void gs_task_del(struct gs_task const *task)
{
    gs_ctx_put(task->ctx, (struct gs_task *)task);
}
