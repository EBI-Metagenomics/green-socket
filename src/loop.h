#ifndef LOOP_H
#define LOOP_H

#include "ev/ev.h"
#include <stdbool.h>

bool gs_loop_init(void);
bool gs_loop_has_work(void);
void gs_loop_work(void);
void gs_loop_stop(void);
void gs_loop_del(void);

#define gs_loop_ev_init(ev, cb_) ev_init((ev), (cb_))
#define gs_loop_ev_io_set(ev, fd_, events_) ev_io_set((ev), (fd_), (events_))

void gs_loop_ev_io_start(struct ev_io *w);
void gs_loop_ev_io_stop(struct ev_io *w);

void gs_loop_ev_timer_again(struct ev_timer *w);
void gs_loop_ev_timer_stop(struct ev_timer *w);

#endif
