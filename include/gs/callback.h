#ifndef GS_CALLBACK_H
#define GS_CALLBACK_H

#include <stdbool.h>
#include <stddef.h>

struct gs_ctx;
struct gs_task;

typedef bool gs_read_fn(struct gs_task *, size_t size, void *buf, size_t *read);
typedef bool gs_write_fn(struct gs_task *, size_t size, void const *buf,
                         size_t *written);

typedef bool gs_read_cb(struct gs_task *, gs_read_fn);
typedef bool gs_write_cb(struct gs_task *, gs_write_fn);
typedef void gs_when_done_cb(struct gs_task *task);

#endif
