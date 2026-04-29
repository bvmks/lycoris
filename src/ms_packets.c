#include <stdlib.h>
#include <string.h>

#include "ms_packets.h"


void packet_init(struct ms_packet* packet)
{
    packet->header = NULL;
    packet->data = NULL;
    packet->data_size = 0;
}


void packet_clear(struct ms_packet* packet)
{
    if(packet->header){
        header_free(packet->header);
        packet->header = NULL;
    }
    if(packet->data) {
        free(packet->data);
        packet->data = NULL;
        packet->data_size = 0;
    }
}

void packet_free(struct ms_packet* packet)
{
    packet_clear(packet);
    free(packet);
}

void packet_copy(struct ms_packet* dst, const struct ms_packet* src)
{
    if(dst == src) 
        return;
    packet_clear(dst);
    header_copy(&dst->header, src->header);
    if(src->data) {
        dst->data = malloc(src->data_size);
        memcpy(dst->data, src->data, src->data_size);
        dst->data_size = src->data_size;
    }
}

void packet_move(struct ms_packet* dst, struct ms_packet* src)
{
    if(dst == src) 
        return;
    packet_clear(dst);
    header_move(&dst->header, src->header);
    if(src->data) {
        dst->data = src->data;
        src->data = NULL;
        dst->data_size = src->data_size;
        src->data_size = 0;
    }
}
