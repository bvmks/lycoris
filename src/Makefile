FLAVOR=default

ifeq ($(FLAVOR),default)

CC = gcc
CONF =
endif

#STATIC = -static
STATIC =
INCPATH = -I../lib
CFLAGS = -O0 -Wall -g $(INCPATH) $(CONF) $(STATIC)

TARGETS = msnode mskndb

#INSTALLPATH = /usr/local/bin/

NODE_OBJS = msnode.o fileutils.o message.o ms_rx.o \
			ms_nodecfg.o ms_sig.o ms_lh.o ms_nodeid.o\
			addrport.o ms_comm_ctx.o socks.o keyutils.o\
			ms_txq.o ms_peers.o hexdata.o addrcol.o\
			ms_kndb.o unitable.o log.o rep.o ms_con.o\
			ms_chandl.o ms_cparser.o trie.o utils.o\
			stupid_peers_parser.o\
			../lib/monocypher/libmonocypher.a \
			../lib/sue/libsue.a

KNDB_OBJS = mskndb.o ms_kndb.o log.o hexdata.o fileutils.o\

#MSCTL_OBJS = msctl.o 


all:	$(TARGETS)

%.o:	%.c
	$(CC) $(CFLAGS) -c $< -o $@

%:	%.o
	$(CC) $(STATIC) $^ -o $@


msnode:	$(NODE_OBJS)

mskndb:	$(KNDB_OBJS)


install:
	cp -v $(TARGETS) $(INSTALLPATH)


../lib/monocypher/libmonocypher.a:
	cd ../lib/monocypher && $(MAKE) CC="$(CC)"

../lib/sue/libsue.a:
	cd ../lib/sue && $(MAKE) CC="$(CC)"

_version.h: Version
	 echo '#define MS_VERSION "'`head -1 Version`'"' > $@

deps.mk:	$(wildcard *.c) _version.h
	$(CC) -MM $(INCPATH) $(wildcard *.c) > $@

clean:
	rm -f *.o $(TARGETS) $(TESTBINS) deps.mk _version.h

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

FORCE:
