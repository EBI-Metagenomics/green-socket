#include "gs/gs.h"
#include "debug.h"
#include "ev/ev.h"
#include "loop.h"

bool gs_init(void) { return gs_loop_init(); }

void gs_stop(void)
{
    gs_loop_stop();
    gs_work();
    gs_loop_del();
}

void gs_work(void)
{
    debug();
    while (gs_loop_has_work())
    {
        debug();
        gs_loop_work();
    }
    debug();
}

void gs_sleep(double seconds) { ev_sleep(seconds); }
