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

void test_send(void)
{
    struct conn *conn = conn_new(echo_ip(), 9090);
    if (!conn) ERROR;

    gs_init();
    if (gs_start()) ERROR;

    struct gs_ctx *ctx = gs_ctx_new(conn_sockfd(conn), 2);
    if (!ctx) ERROR;

    gs_ctx_del(ctx);

    conn_del(conn);

    gs_stop();
}

int main(void)
{
    test_start_stop();
    test_send();
    return 0;
}
