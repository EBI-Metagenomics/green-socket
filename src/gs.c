#include "gs/gs.h"
#include "ev/ev.h"
#include "loop.h"

bool gs_init(void) { return gs_loop_init(); }

void gs_stop(void) {}

void gs_work(void)
{
    while (gs_loop_has_work())
        gs_loop_work();
}

void gs_sleep(double seconds) { ev_sleep(seconds); }
