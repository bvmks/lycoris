#include "ms_pqueue.h"
#include <stdlib.h>

void pqueue_init(struct ms_pqueue *queue)
{
    queue->head = NULL;
    queue->head->next = NULL;
    queue->tail = NULL;
    queue->tail->next = NULL;
    queue->size = 0;
}


void pqueue_free(struct ms_pqueue* queue)
{
    struct ms_pqueue_node* tmp = queue->head;
    if (tmp == queue->tail) {
        packet_free(&(tmp->packet.packet));
        free(queue);
        return;
    }
    for (;tmp != queue->tail; tmp = tmp->next) {
        packet_free(&(tmp->packet.packet));
    }
    pqueue_init(queue);
}

int pqueue_push(struct ms_pqueue* queue, const struct ms_received_packet* src)
{
    if (queue->size == 0) {
        queue->head = malloc(sizeof(struct ms_pqueue_node));
        packet_copy(&queue->head->packet.packet, &src->packet);
        queue->tail = queue->head;
        queue->size = 1;
        return 0;
    }

    queue->tail->next = malloc(sizeof(struct ms_pqueue_node));
    packet_copy(&queue->tail->next->packet.packet, &src->packet);
    queue->tail->next->packet.src_addr = src->src_addr;
    queue->tail = queue->tail->next;
    queue->size++;
    return 0;
}

int pqueue_pop (struct ms_pqueue* queue, struct ms_received_packet* dst)
{
    packet_copy(&dst->packet, &queue->head->packet.packet);
    dst->src_addr = queue->head->packet.src_addr;
    struct ms_pqueue_node* tmp = queue->head;
    packet_free(&tmp->packet.packet);
    queue->head = queue->head->next;
    return 0;
}

int pqueue_peek(const struct ms_pqueue* queue, struct ms_received_packet* dst, unsigned int pos)
{
    int i;
    struct ms_pqueue_node* tmp = queue->head;
    if (pos < 0 || pos > queue->size)
        return -1;

    for (i = 0; i < pos; i++) {
        tmp = tmp->next;
    }

    packet_copy(&dst->packet, &tmp->packet.packet);
    dst->src_addr = tmp->packet.src_addr;

    return 0;
}
