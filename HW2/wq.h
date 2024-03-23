#ifndef __WQ__
#define __WQ__

#include <pthread.h>


typedef struct wq_item {
    int client_socket_fd;
    struct wq_item *next;
    struct wq_item *prev;
} wq_item_t;
typedef struct wq {
    int size;
    wq_item_t *head;
} wq_t;

void wq_init(wq_t *wq);

void wq_push(wq_t *wq, int client_socket_fd);

int wq_pop(wq_t *wq);

#endif
