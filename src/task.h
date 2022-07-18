#ifndef TASK_H
#define TASK_H

#include "cco/cco.h"
#include "ev/ev.h"
#include "gs/callback.h"
#include <stdbool.h>

struct gs_ctx;

struct gs_task
{
    void *data;

    struct gs_ctx *ctx;

    bool done;
    bool cancelled;
    int errno_value;

    gs_read_cb *read_cb;
    gs_write_cb *write_cb;
    gs_when_done_cb *when_done_cb;

    struct
    {
        ev_io read;
        ev_io write;
        ev_timer timeout;
    } watcher;

    double timeout;
    struct cco_node node;
};

void gs_task_init(struct gs_task *, double timeout);
void gs_task_start(struct gs_task *);

#endif
