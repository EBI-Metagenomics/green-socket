#include "task.h"
#include "ctx.h"
#include "die.h"
#include "gs.h"
#include "gs/rc.h"
#include "gs/task.h"
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

    if (!task->read_cb(task, read_fn))
    {
        ev_timer_again(loop, &task->watcher.timeout);
    }
    else
    {
        ev_io_stop(loop, &task->watcher.read);
        ev_timer_stop(loop, &task->watcher.timeout);
        task->when_done_cb(task);
    }
}

static void write_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    (void)loop;
    if (EV_ERROR & revents) die("invalid event during write");
    struct gs_task *task = w->data;

    if (!task->write_cb(task, write_fn))
    {
        ev_timer_again(loop, &task->watcher.timeout);
    }
    else
    {
        ev_io_stop(loop, &task->watcher.read);
        ev_timer_stop(loop, &task->watcher.timeout);
        task->when_done_cb(task);
    }
}

static void timeout_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
    (void)loop;
    if (EV_ERROR & revents) die("invalid event during timeout");
    struct gs_task *task = w->data;

    ev_io_stop(loop, &task->watcher.read);
    ev_io_stop(loop, &task->watcher.write);
    ev_timer_stop(loop, &task->watcher.timeout);
    task->when_done_cb(task);
}

void gs_task_init(struct gs_task *task, double timeout)
{
    task->done = false;
    task->cancelled = false;
    task->errno_value = 0;

    task->read_cb = 0;
    task->write_cb = 0;
    task->when_done_cb = 0;

    ev_io_init(&task->watcher.read, read_cb, 0, EV_READ);
    ev_io_init(&task->watcher.write, write_cb, 0, EV_WRITE);
    ev_init(&task->watcher.timeout, timeout_cb);

    task->watcher.read.data = task;
    task->watcher.write.data = task;
    task->watcher.timeout.data = task;

    task->timeout = timeout;
    cco_node_init(&task->node);
}

void gs_task_start(struct gs_task *task)
{
    task->watcher.timeout.repeat = task->timeout;
    ev_timer_again(loop, &task->watcher.timeout);
}

bool gs_task_done(struct gs_task const *task) { return task->done; }

void gs_task_cancel(struct gs_task *task)
{
    ev_io_stop(loop, &task->watcher.read);
    ev_io_stop(loop, &task->watcher.write);
    ev_timer_stop(loop, &task->watcher.timeout);
    task->cancelled = true;
}

bool gs_task_cancelled(struct gs_task const *task) { return task->cancelled; }

int gs_task_errno(struct gs_task const *task) { return task->errno_value; }

void gs_task_del(struct gs_task const *task)
{
    gs_ctx_put(task->ctx, (struct gs_task *)task);
}
