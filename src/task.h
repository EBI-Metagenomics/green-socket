#ifndef TASK_H
#define TASK_H

#include "cco/cco.h"
#include "gs/callback.h"
#include "libev.h"
#include <stdatomic.h>
#include <stdbool.h>

struct gs_ctx;

enum gs_task_type
{
    GS_TASK_NOTSET,
    GS_TASK_SEND,
    GS_TASK_RECV,
};

struct gs_task
{
    void *data;

    struct gs_ctx *ctx;

    bool active;
    bool done;
    atomic_flag cancel_once;
    bool cancelled;
    int errno_value;

    enum gs_task_type type;

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

void gs_task_init(struct gs_task *, struct gs_ctx *);
void gs_task_reset(struct gs_task *, double timeout);
void gs_task_setup_send(struct gs_task *, void *data, gs_write_cb *,
                        gs_when_done_cb *);
void gs_task_setup_recv(struct gs_task *, void *data, gs_read_cb *,
                        gs_when_done_cb *);
void gs_task_start(struct gs_task *);

#endif
