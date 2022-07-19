#include "gs/gs.h"
#include "ctx.h"
#include "ev/ev.h"
#include "task.h"
#include "thread.h"

struct ev_loop *loop = 0;

static void thread_start(void) { ev_run(loop, 0); }

void gs_init(void) { loop = EV_DEFAULT; }

enum gs_rc gs_start(void) { return gs_thread_create(&thread_start); }

void gs_stop(void)
{
    ev_break(loop, EVBREAK_ALL);
    gs_thread_join();
}

void gs_sleep(double seconds) { ev_sleep(seconds); }
