#include "gs/gs.h"
#include "conn.h"
#include "helper.h"

static char const *echo_ip(void)
{
    char const *addr = getenv("GS_ECHO_IP");
    return addr ? addr : "127.0.0.1";
}

void test_start_stop(void)
{
    gs_init();
    if (gs_start()) ERROR;
    gs_sleep(0.1);
    gs_stop();
}

static enum gs_rc write_cb(struct gs_task *task, gs_write_fn *write_fn)
{
    printf("test::write_cb\n");
    static char const hello[] = "hello";

    size_t sz = 0;
    enum gs_rc rc = 0;

    size_t size = sizeof hello;
    char const *p = hello;
    while (!(rc = (*write_fn)(task, size, p, &sz)))
    {
        p += sz;
        size = (size_t)(size - sz);
    }

    printf("RC: %d\n", rc);
    printf("SIZE: %ld\n", size);
    gs_sleep(1.0);
    return rc;
}

static void when_done_cb(struct gs_task *task)
{
    (void)task;
    printf("test::when_done_cb\n");
}

void test_send_timeout(void)
{
    struct conn *conn = conn_new("127.0.0.1", 8723);
    if (!conn) ERROR;
    int sockfd = conn_sockfd(conn);

    gs_init();
    if (gs_start()) ERROR;

    struct gs_ctx *ctx = gs_ctx_new(sockfd, 2);
    if (!ctx) ERROR;

    struct gs_task *task = gs_ctx_send(ctx, 0, &write_cb, &when_done_cb, 0.1);

    gs_sleep(0.3);
    if (!gs_task_done(task)) ERROR;
    if (!gs_task_cancelled(task)) ERROR;

    gs_ctx_del(ctx);
    gs_stop();

    conn_del(conn);
}

int main(void)
{
    test_start_stop();
    test_send_timeout();
    return 0;
}
