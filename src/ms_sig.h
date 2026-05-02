#ifndef _MS_SIG_H
#define _MS_SIG_H

struct sue_event_selector;
struct ms_udp_receiver;

struct ms_signal_target;

struct ms_signal_target* prepare_sig_handlers(struct sue_event_selector* s,
                                              struct ms_udp_receiver* rx);

#endif
