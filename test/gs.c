#include "gs/gs.h"
#include "conn.h"
#include "helper.h"

static char const *echo_ip(void)
{
    char const *addr = getenv("GS_ECHO_IP");
    return addr ? addr : "127.0.0.1";
}

static unsigned echo_port(void)
{
    char const *port = getenv("GS_ECHO_PORT");
    return port ? (unsigned)atoi(port) : 9090;
}

void test_start_stop(void)
{
    if (!gs_start()) ERROR;
    gs_stop();

    if (!gs_start()) ERROR;
    gs_work();
    gs_stop();
}

static bool write_cb_called = false;
static enum gs_rc write_cb(struct gs_task *task, gs_write_fn *write_fn)
{
    write_cb_called = true;
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
        if (size == 0)
        {
            rc = GS_EOF;
            break;
        }
    }

    return rc;
}

static bool when_done_cb_called = false;
static void when_done_cb(struct gs_task *task)
{
    when_done_cb_called = true;
    (void)task;
    printf("test::when_done_cb\n");
}

void test_send_timeout(void)
{
    struct conn *conn = conn_new("127.0.0.1", 8723);
    if (!conn) ERROR;
    int sockfd = conn_sockfd(conn);

    if (!gs_start()) ERROR;

    struct gs_ctx *ctx = gs_ctx_new(sockfd, 2);
    if (!ctx) ERROR;

    write_cb_called = false;
    when_done_cb_called = false;

    struct gs_task *task = gs_ctx_send(ctx, 0, &write_cb, &when_done_cb, 0.001);
    if (gs_task_done(task)) ERROR;
    if (gs_task_cancelled(task)) ERROR;

    for (unsigned i = 0; i < 5; ++i)
    {
        gs_sleep(0.001);
        (void)gs_work();
    }
    if (!gs_task_done(task)) ERROR;
    if (!gs_task_cancelled(task)) ERROR;

    gs_ctx_del(ctx);
    gs_stop();

    conn_del(conn);

    if (write_cb_called) ERROR;
    if (!when_done_cb_called) ERROR;
}

void test_send_successfully(void)
{
    struct conn *conn = conn_new(echo_ip(), echo_port());
    if (!conn) ERROR;
    if (!conn_connect(conn))
    {
        conn_del(conn);
        ERROR;
    }
    int sockfd = conn_sockfd(conn);

    if (!gs_start()) ERROR;

    struct gs_ctx *ctx = gs_ctx_new(sockfd, 2);
    if (!ctx) ERROR;

    write_cb_called = false;
    when_done_cb_called = false;

    struct gs_task *task = gs_ctx_send(ctx, 0, &write_cb, &when_done_cb, 5.0);

    for (unsigned i = 0; i < 5; ++i)
    {
        gs_sleep(0.1);
        (void)gs_work();
    }
    if (!gs_task_done(task)) ERROR;
    if (gs_task_cancelled(task)) ERROR;

    gs_ctx_del(ctx);
    gs_stop();

    conn_del(conn);

    if (!write_cb_called) ERROR;
    if (!when_done_cb_called) ERROR;
}

int main(void)
{
    test_start_stop();
    test_send_timeout();
    test_send_successfully();
    return 0;
}
