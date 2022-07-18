#ifndef GS_CALLBACK_H
#define GS_CALLBACK_H

#include <stddef.h>

struct gs_ctx;
struct gs_task;

typedef enum gs_rc gs_read_fn(struct gs_task *, size_t sz, void *buf,
                              size_t *size_read);
typedef enum gs_rc gs_write_fn(struct gs_task *, size_t sz, void const *buf,
                               size_t *size_written);

typedef enum gs_rc gs_read_cb(struct gs_task *, gs_read_fn);
typedef enum gs_rc gs_write_cb(struct gs_task *, gs_write_fn);
typedef void gs_when_done_cb(struct gs_task *task);

#endif
