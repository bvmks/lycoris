
#include "ms_packets.h"
#include <stdlib.h>


void packet_init(struct ms_packet* packet)
{
    packet->data = NULL;
    packet->data_size = 0;
}

void packet_free(struct ms_packet* packet)
{
    if(packet->data) 
        free(packet->data);
    packet->data = NULL;
    packet->data_size = 0;
}
