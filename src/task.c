#include "task.h"
#include "ctx.h"
#include "gs/task.h"
#include "socket_include.h"
#include <errno.h>
#include <stdlib.h>

static bool read_fn(struct gs_task *task, size_t size, void *buf,
                    size_t *size_read)
{
    task->errno_value = 0;

    ssize_t sz = read(gs_ctx_socket(task->ctx), buf, size);
    if (sz == -1)
    {
        if (errno == EWOULDBLOCK)
        {
            // Turn timeout off?
            *size_read = 0;
            return true;
        }
        task->errno_value = errno;
        return false;
    }
    if ((size_t)sz == size)
    {
        // Turn timeout off?
    }
    else
    {
        // Restat timeout?
    }
    *size_read = sz;
    return true;
}

static void read_cb(EV_P_ ev_io *w, int revents)
{
    struct gs_task *task = w->data;
    task->read_cb(task, read_fn);
    ev_io_stop(EV_A_ w);
    ev_break(EV_A_ EVBREAK_ALL);
}

static void timeout_cb(EV_P_ ev_timer *w, int revents)
{
    puts("timeout");
    // this causes the innermost ev_run to stop iterating
    ev_break(EV_A_ EVBREAK_ONE);
}

void gs_task_init(struct gs_task *task, double timeout)
{
    task->done = false;
    task->cancelled = false;
    task->errno_value = 0;

    task->write_fn = 0;
    task->when_done_fn = 0;

    cco_node_init(&task->node);

    ev_io_init(&task->watcher.read, read_cb, 0, EV_READ);
    ev_io_init(&task->watcher.write, write_cb, 0, EV_WRITE);
    ev_timer_init(&task->watcher.timeout, timeout_cb, 0.0, 0.0);

    task->watcher.read.data = task;
    task->watcher.write.data = task;
    task->watcher.timeout.data = task;

    ev_timer_set(&task->watcher.timeout, timeout, 0.0);
}

bool gs_task_done(struct gs_task const *task) { return task->done; }

void gs_task_cancel(struct gs_task *task);

bool gs_task_cancelled(struct gs_task const *task) { return task->cancelled; }

int gs_task_errno(struct gs_task const *task) { return task->errno_value; }

void gs_task_del(struct gs_task const *task) { free((void *)task); }

void gs_task_setup_watchers()
{
    // ev_io_init (&stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    // ev_io_start (loop, &stdin_watcher);
    //
    // // initialise a timer watcher, then start it
    // // simple non-repeating 5.5 second timeout
    // ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 0.);
    // ev_timer_start (loop, &timeout_watcher);
}
