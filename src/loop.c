#include "loop.h"
#include "die.h"
#include "gs/rc.h"
#include "libev.h"
#include "sync.h"
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

static struct sync *sync = 0;
static atomic_bool stop = false;
static ev_async async_watcher = {0};
static atomic_bool pending_work = false;

static void thread_start(void)
{
    gs_sync_lock(sync);
    ev_run(0);
    gs_sync_unlock(sync);
}

static void invoke_pending_work(void)
{
    while (ev_pending_count())
    {
        atomic_store(&pending_work, true);
        gs_sync_wait_signal(sync);
    }
}

static void async_cb(ev_async *w, int revents);

void lock_cb(void) { gs_sync_lock(sync); }

void unlock_cb(void) { gs_sync_unlock(sync); }

bool gs_loop_start(void)
{
    ev_default_loop(0);
    atomic_store(&stop, false);
    ev_async_init(&async_watcher, async_cb);
    ev_async_start(&async_watcher);

    ev_set_invoke_pending_cb(invoke_pending_work);
    ev_set_loop_release_cb(unlock_cb, lock_cb);

    if (!(sync = gs_sync_init(thread_start)))
    {
        ev_async_stop(&async_watcher);
        return false;
    }

    return true;
}

bool gs_loop_has_work(void) { return atomic_load(&pending_work); }

void gs_loop_work(void)
{
    gs_sync_lock(sync);
    ev_invoke_pending();
    atomic_store(&pending_work, false);
    gs_sync_signal(sync);
    gs_sync_unlock(sync);
}

void gs_loop_stop(void)
{
    gs_sync_lock(sync);
    atomic_store(&stop, true);
    ev_async_send(&async_watcher);
    gs_sync_unlock(sync);
    while (!gs_loop_has_work())
        ev_sleep(0.01);
    gs_loop_work();
    ev_sleep(0.01);
    gs_loop_work();
    ev_sleep(0.01);
    gs_loop_work();
    ev_sleep(0.01);
    gs_loop_work();
    ev_sleep(0.01);
    gs_loop_work();
    gs_sync_join(sync);
}

void gs_loop_io_start(struct ev_io *w)
{
    gs_sync_lock(sync);
    ev_io_start(w);
    ev_async_send(&async_watcher);
    gs_sync_unlock(sync);
}

void gs_loop_io_stop_within(struct ev_io *w) { ev_io_stop(w); }

void gs_loop_timer_again(struct ev_timer *w)
{
    gs_sync_lock(sync);
    ev_timer_again(w);
    ev_async_send(&async_watcher);
    gs_sync_unlock(sync);
}

void gs_loop_timer_again_within(struct ev_timer *w) { ev_timer_again(w); }

void gs_loop_timer_stop_within(struct ev_timer *w) { ev_timer_stop(w); }

static void async_cb(ev_async *w, int events)
{
    (void)w;
    if (EV_ERROR & events) die("invalid event during async_cb");
    if (atomic_load(&stop))
    {
        ev_async_stop(w);
        ev_break(EVBREAK_ONE);
    }
}
