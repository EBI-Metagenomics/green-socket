#include "gs/gs.h"
#include "libev.h"
#include "loop.h"

bool gs_start(void) { return gs_loop_start(); }

void gs_stop(void) { gs_loop_stop(); }

bool gs_work(void) { return gs_loop_has_work() ? gs_loop_work(), true : false; }

void gs_sleep(double seconds) { ev_sleep(seconds); }
