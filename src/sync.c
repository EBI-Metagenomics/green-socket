#include "sync.h"
#include <pthread.h>
#include <stdbool.h>

struct sync
{
    pthread_t tid;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    void (*start_fn)(void);
};

static bool mutex_init(pthread_mutex_t *);
static void mutex_del(pthread_mutex_t *);

static bool cond_init(pthread_cond_t *);
static void cond_del(pthread_cond_t *);

static void sync_cleanup(struct sync const *sync);

static void *start_routine(void *arg)
{
    struct sync *sync = arg;
    sync->start_fn();
    sync_cleanup(sync);
    pthread_exit(0);
    return 0;
}

struct sync *gs_sync_init(void (*start_fn)(void))
{
    static struct sync sync = {0};

    sync.tid = 0;

    if (!mutex_init(&sync.lock)) return 0;

    if (!cond_init(&sync.cond))
    {
        mutex_del(&sync.lock);
        return 0;
    }

    sync.start_fn = start_fn;

    if (pthread_create(&sync.tid, 0, start_routine, &sync))
    {
        sync_cleanup(&sync);
        return 0;
    }

    return &sync;
}

void gs_sync_lock(struct sync *sync) { (void)pthread_mutex_lock(&sync->lock); }

void gs_sync_unlock(struct sync *sync)
{
    (void)pthread_mutex_unlock(&sync->lock);
}

void gs_sync_wake(struct sync *sync)
{
    (void)pthread_cond_signal(&sync->cond);
}

void gs_sync_wait(struct sync *sync)
{
    (void)pthread_cond_wait(&sync->cond, &sync->lock);
}

void gs_sync_join(struct sync *sync) { (void)pthread_join(sync->tid, 0); }

static void sync_cleanup(struct sync const *sync)
{
    struct sync *s = (struct sync *)sync;
    mutex_del(&s->lock);
    cond_del(&s->cond);
}

static bool mutex_init(pthread_mutex_t *mutex)
{
    return pthread_mutex_init(mutex, 0) ? false : true;
}

static void mutex_del(pthread_mutex_t *mutex)
{
    (void)pthread_mutex_destroy(mutex);
}

static bool cond_init(pthread_cond_t *cond)
{
    return pthread_cond_init(cond, 0) ? false : true;
}

static void cond_del(pthread_cond_t *cond) { (void)pthread_cond_destroy(cond); }
