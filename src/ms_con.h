#ifndef _MS_CONTROL_H
#define _MS_CONTROL_H

struct sue_event_selector;
struct ms_node_cfg;
struct ms_udp_receiver;

struct ms_control_receiver;

struct ms_control_receiver *
launch_control_receiver(struct sue_event_selector *sel,
                           struct ms_node_cfg *cfg,
                           struct ms_udp_receiver *rx);

void dispose_control_receiver(struct ms_control_receiver *crx);

#endif
