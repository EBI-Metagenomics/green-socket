#include "loop.h"
#include "die.h"
#include "ev/ev.h"
#include "gs/rc.h"
#include "sync.h"
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

static struct ev_loop *loop = 0;
static struct sync *sync = 0;
static ev_async async_watcher = {0};
static atomic_bool pending_work = false;

#include <stdio.h>
static void thread_start(void)
{
    printf("loop:thread_start 1\n");
    fflush(stdout);
    gs_sync_lock(sync);
    printf("loop:thread_start 2\n");
    fflush(stdout);
    ev_run(loop, 0);
    printf("loop:thread_start 3\n");
    fflush(stdout);
    gs_sync_unlock(sync);
    printf("loop:thread_start 4\n");
    fflush(stdout);
}

static void invoke_pending_work(struct ev_loop *loop)
{
    while (ev_pending_count(loop))
    {
        atomic_store(&pending_work, true);
        gs_sync_wait_signal(sync);
        atomic_store(&pending_work, false);
    }
}

static void async_cb(struct ev_loop *loop, ev_async *w, int revents);

void lock_cb(struct ev_loop *loop)
{
    (void)loop;
    gs_sync_lock(sync);
}

void unlock_cb(struct ev_loop *loop)
{
    (void)loop;
    gs_sync_unlock(sync);
}

bool gs_loop_init(void)
{
    loop = EV_DEFAULT;
    ev_async_init(&async_watcher, async_cb);
    ev_async_start(loop, &async_watcher);

    ev_set_invoke_pending_cb(loop, invoke_pending_work);
    ev_set_loop_release_cb(loop, unlock_cb, lock_cb);

    if (!(sync = gs_sync_init(thread_start)))
    {
        ev_async_stop(loop, &async_watcher);
        return false;
    }

    return true;
}

bool gs_loop_has_work(void) { return atomic_load(&pending_work); }

void gs_loop_work(void)
{
    gs_sync_lock(sync);
    ev_invoke_pending(loop);
    gs_sync_signal(sync);
    gs_sync_unlock(sync);
}

void gs_loop_cleanup(void) { gs_sync_cleanup(sync); }

void gs_loop_ev_io_start(struct ev_io *w)
{
    gs_sync_lock(sync);
    ev_io_start(loop, w);
    ev_async_send(loop, &async_watcher);
    gs_sync_unlock(sync);
}

void gs_loop_ev_io_stop(struct ev_io *w)
{
    gs_sync_lock(sync);
    ev_io_stop(loop, w);
    ev_async_send(loop, &async_watcher);
    gs_sync_unlock(sync);
}

void gs_loop_ev_timer_again(struct ev_timer *w)
{
    gs_sync_lock(sync);
    ev_timer_again(loop, w);
    ev_async_send(loop, &async_watcher);
    gs_sync_unlock(sync);
}

void gs_loop_ev_timer_stop(struct ev_timer *w)
{
    gs_sync_lock(sync);
    ev_timer_stop(loop, w);
    ev_async_send(loop, &async_watcher);
    gs_sync_unlock(sync);
}

static void async_cb(struct ev_loop *l, ev_async *w, int events)
{
    (void)l;
    (void)w;
    if (EV_ERROR & events) die("invalid event during async_cb");
}
