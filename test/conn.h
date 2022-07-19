#ifndef CONN_H
#define CONN_H

#include <stdbool.h>

struct conn;

struct conn *conn_new(char const *ip, unsigned port);
int conn_sockfd(struct conn const *conn);
bool conn_connect(struct conn *conn);
void conn_del(struct conn const *conn);

#endif
