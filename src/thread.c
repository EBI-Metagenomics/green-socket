#include "thread.h"
#include "gs/rc.h"
#include <pthread.h>

static struct
{
    pthread_t thread_id;
    gs_start_fn_t *start_fn;
} g = {0};

static void *thread_start(void *ptr)
{
    (void)ptr;
    (*g.start_fn)();
    pthread_exit(0);
    return 0;
}

enum gs_rc gs_thread_create(gs_start_fn_t *start_fn)
{
    g.thread_id = 0;
    g.start_fn = start_fn;
    return pthread_create(&g.thread_id, 0, &thread_start, 0) ? GS_ERROR : GS_OK;
}

void gs_thread_join(void) { (void)pthread_join(g.thread_id, 0); }
