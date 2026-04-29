#include "ms_pqueue.h"
#include <stdlib.h>

void pqueue_init(struct ms_pqueue *queue)
{
    queue->head = queue->tail = NULL;
    queue->size = 0;
}

void pqueue_free(struct ms_pqueue* queue)
{
    struct ms_pqueue_node* tmp = queue->head;
    if (tmp == queue->tail) {
        packet_free(&(tmp->packet));
        free(tmp);
        return;
    }
    for (;tmp != queue->tail; tmp = tmp->next) {
        packet_free(&(tmp->packet));
    }
    free(tmp);
}

int pqueue_push(struct ms_pqueue* queue, const struct ms_packet* src)
{
    if (queue->size == 0) {
        queue->head = malloc(sizeof(struct ms_pqueue_node));
        packet_copy(&queue->head->packet, src);
        queue->tail = queue->head;
    }
    else if (queue->size == pqueue_max_size) {
        pqueue_pop(queue, NULL);
        pqueue_push(queue, src);
        return 0; /* maybe oshibka, chance 2% */
    }
    else {
        queue->tail->next = malloc(sizeof(struct ms_pqueue_node));
        packet_copy(&queue->tail->next->packet, src);
        queue->tail = queue->tail->next;
        queue->tail->next = NULL;
    }
    queue->size++;
    return 0;
}

int pqueue_pop(struct ms_pqueue* queue, struct ms_packet* dst)
{
    if (dst != NULL) {
        packet_move(dst, &queue->head->packet);
    }

    packet_free(&queue->head->packet);
    free(queue->head);
    if (queue->head != queue->tail) {
        queue->head = queue->head->next;
    }
    queue->size--;

    return 0;
}

int pqueue_get(const struct ms_pqueue* queue, struct ms_packet* dst, unsigned int pos)
{
    int i;
    struct ms_pqueue_node* tmp = queue->head;
    if (pos < 0 || pos > queue->size)
        return -1;
    for (i = 0; i < pos; i++) {
        tmp = tmp->next;
    }
    packet_copy(dst, &tmp->packet);
    return 0;
}

struct ms_packet* pqueue_peek(const struct ms_pqueue* queue, unsigned int pos)
{
    int i;
    struct ms_pqueue_node* tmp = queue->head;
    if (pos < 0 || pos > queue->size)
        return NULL;
    for (i = 0; i < pos; i++) {
        tmp = tmp->next;
    }
    return &tmp->packet;
}

