#ifndef _MS_LOOP_HOOKS_H
#define _MS_LOOP_HOOKS_H

struct sue_event_selector;
struct ms_udp_receiver;

struct ms_loophook_target;

struct ms_loophook_target* prepare_loophooks(struct sue_event_selector* s,
                                             struct ms_udp_receiver* node);

#endif
