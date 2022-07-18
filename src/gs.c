#include "gs/gs.h"
#include "ctx.h"
#include "ev/ev.h"
#include "task.h"

struct ev_loop *loop = 0;

void gs_init(void) { loop = EV_DEFAULT; }

void gs_run(void) { ev_run(loop, 0); }

void gs_stop(void) { ev_break(loop, EVBREAK_ALL); }
