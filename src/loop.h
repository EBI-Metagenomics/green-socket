#ifndef LOOP_H
#define LOOP_H

#include "libev.h"
#include <stdbool.h>

bool gs_loop_start(void);
bool gs_loop_has_work(void);
void gs_loop_work(void);
void gs_loop_stop(void);

#define gs_loop_init(ev, cb_) ev_init((ev), (cb_))
#define gs_loop_io_set(ev, fd_, events_) ev_io_set((ev), (fd_), (events_))

void gs_loop_io_start(struct ev_io *w);
void gs_loop_io_stop_within(struct ev_io *w);

void gs_loop_timer_again(struct ev_timer *w);
void gs_loop_timer_again_within(struct ev_timer *w);
void gs_loop_timer_stop_within(struct ev_timer *w);

#endif
