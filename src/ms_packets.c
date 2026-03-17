#include <stdlib.h>
#include <string.h>

#include "ms_packets.h"


void packet_init(struct ms_packet* packet)
{
    header_init(&packet->header);
    packet->data = NULL;
    packet->data_size = 0;
}

void packet_free(struct ms_packet* packet)
{
    if(packet->data) 
        free(packet->data);
    packet_init(packet);
}

void packet_copy(struct ms_packet* dst, const struct ms_packet* src)
{
    dst->header = src->header;
    packet_free(dst);
    if(src->data) {
        dst->data = malloc(src->data_size);
        memcpy(dst->data, src->data, src->data_size);
        dst->data_size = src->data_size;
    }
}

void packet_move(struct ms_packet* dst, struct ms_packet* src)
{
    dst->header = src->header;
    packet_free(dst);
    if(src->data) {
        dst->data = src->data;
        dst->data_size = src->data_size;
    }
    packet_free(src);
}
