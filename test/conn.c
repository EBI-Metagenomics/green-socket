#include "conn.h"
#include "socket_include.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum addr_ipv
{
    IPV4,
    IPV6
};

struct conn
{
    struct sockaddr_in addr;
    int sockfd;
};

static int addr_setup(struct sockaddr_in *addr, enum addr_ipv ipv,
                      char const *ip, uint16_t port)
{
    addr->sin_family = ipv == IPV4 ? AF_INET : AF_INET6;
    addr->sin_port = htons(port);
    if ((inet_pton(addr->sin_family, ip, &addr->sin_addr)) != 1) return -1;
    return 0;
}

static inline bool set_socket_nonblocking(int sockfd)
{
    return fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK) != -1;
}

static inline bool set_socket_reuseaddr(int sockfd)
{
    int const y = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(y)) != -1;
}

static inline bool set_socket_linger(int sockfd, int seconds)
{
    struct linger linger = {.l_onoff = 1, .l_linger = seconds};
    socklen_t size = sizeof(linger);
    return setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &linger, size) != -1;
}

struct conn *conn_new(char const *ip, unsigned port)
{
    struct conn *conn = malloc(sizeof(struct conn));
    if (!conn) return 0;

    if ((addr_setup(&conn->addr, IPV4, ip, port)))
    {
        perror("failed to setup addr");
        free(conn);
        return false;
    }

    int const sockfd = socket(conn->addr.sin_family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        perror("failed to create socket");
        free(conn);
        return false;
    }

    if (!set_socket_reuseaddr(sockfd))
    {
        perror("failed to set socket option");
        goto cleanup;
    }

    if (!set_socket_nonblocking(sockfd))
    {
        perror("failed to set socket nonblocking");
        goto cleanup;
    }

    if (!set_socket_linger(sockfd, 1))
    {
        perror("failed to set socket option");
        goto cleanup;
    }

    conn->sockfd = sockfd;
    return conn;

cleanup:
    if (close(sockfd) == -1) perror("failed to close socket");
    free(conn);
    return 0;
}

int conn_sockfd(struct conn const *conn) { return conn->sockfd; }

bool conn_connect(struct conn *conn)
{
    socklen_t size = sizeof(conn->addr);
    if (connect(conn->sockfd, (struct sockaddr *)&conn->addr, size) != -1)
        return true;

    if (errno == EAGAIN || errno == EINPROGRESS) return true;

    if (close(conn->sockfd) == -1) perror("failed to close socket: ");
    return false;
}

void conn_del(struct conn const *conn)
{
    if (close(conn->sockfd) == -1) perror("failed to close socket: ");
    free((void *)conn);
}
