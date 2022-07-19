#ifndef THREAD_H
#define THREAD_H

typedef void gs_start_fn_t(void);

enum gs_rc gs_thread_create(gs_start_fn_t *start_fn);
void gs_thread_join(void);

#endif
