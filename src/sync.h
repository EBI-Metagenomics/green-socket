#ifndef SYNC_H
#define SYNC_H

#include <stdbool.h>

struct sync *gs_sync_init(void (*start_routine)(void));
void gs_sync_lock(struct sync *sync);
void gs_sync_unlock(struct sync *sync);
void gs_sync_signal(struct sync *sync);
void gs_sync_wait_signal(struct sync *sync);
void gs_sync_join(struct sync *sync);

#endif
