#ifndef GS_TASK_H
#define GS_TASK_H

#include "gs/export.h"
#include <stdbool.h>

struct gs_task;

GS_API bool gs_task_done(struct gs_task const *);
GS_API void gs_task_cancel(struct gs_task *);
GS_API bool gs_task_cancelled(struct gs_task const *);
GS_API int gs_task_errno(struct gs_task const *);

#endif
