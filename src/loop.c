#include "loop.h"
#include "die.h"
#include "gs/rc.h"
#include "libev.h"
#include "sync.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

static struct sync *sync = 0;
static bool stop_loop = false;
static ev_async async_watcher = {0};
static bool pending_work = false;

#define CRITICAL_SECTION_START gs_sync_lock(sync)
#define CRITICAL_SECTION_END gs_sync_unlock(sync)

static void thread_start(void)
{
    CRITICAL_SECTION_START;
    while (!stop_loop)
        ev_run(EVRUN_NOWAIT);
    CRITICAL_SECTION_END;
}

// This has to be called inside the critical section.
static void invoke_pending_cb(void)
{
    while (!stop_loop && ev_pending_count())
    {
        pending_work = true;
        gs_sync_wait(sync);
    }
}

static void async_cb(ev_async *w, int revents);

void lock_cb(void) { gs_sync_lock(sync); }

void unlock_cb(void) { gs_sync_unlock(sync); }

bool gs_loop_start(void)
{
    ev_default_loop(0);
    stop_loop = false;
    ev_async_init(&async_watcher, async_cb);
    ev_async_start(&async_watcher);

    // This makes sure invoke_pending_work is called
    // inside the critical section.
    ev_set_invoke_pending_cb(invoke_pending_cb);
    ev_set_loop_release_cb(unlock_cb, lock_cb);

    if (!(sync = gs_sync_init(thread_start)))
    {
        ev_async_stop(&async_watcher);
        return false;
    }

    return true;
}

bool gs_loop_has_work(void)
{
    CRITICAL_SECTION_START;
    bool pend = pending_work;
    CRITICAL_SECTION_END;
    return pend;
}

void gs_loop_work(void)
{
    CRITICAL_SECTION_START;
    ev_invoke_pending();
    pending_work = false;
    gs_sync_wake(sync);
    CRITICAL_SECTION_END;
}

void gs_loop_stop(void)
{
    CRITICAL_SECTION_START;
    ev_async_stop(&async_watcher);
    ev_set_invoke_pending_cb(ev_invoke_pending);
    ev_set_loop_release_cb(0, 0);
    stop_loop = true;
    gs_sync_wake(sync);
    CRITICAL_SECTION_END;
    gs_sync_join(sync);
}

void gs_loop_io_start(struct ev_io *w)
{
    CRITICAL_SECTION_START;
    ev_io_start(w);
    ev_async_send(&async_watcher);
    CRITICAL_SECTION_END;
}

// This has to be called inside the critical section.
void gs_loop_io_stop_within(struct ev_io *w) { ev_io_stop(w); }

void gs_loop_timer_again(struct ev_timer *w)
{
    CRITICAL_SECTION_START;
    ev_timer_again(w);
    ev_async_send(&async_watcher);
    CRITICAL_SECTION_END;
}

// This has to be called inside the critical section.
void gs_loop_timer_again_within(struct ev_timer *w) { ev_timer_again(w); }

// This has to be called inside the critical section.
void gs_loop_timer_stop_within(struct ev_timer *w) { ev_timer_stop(w); }

// This has to be called inside the critical section.
static void async_cb(ev_async *w, int events)
{
    (void)w;
    if (EV_ERROR & events) die("invalid event during async_cb");
}
