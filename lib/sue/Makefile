CC = gcc
CXX = g++
CFLAGS = -g -Wall -O0
CXXFLAGS = $(CFLAGS)

CTAGS = ctags

# SOURCES = sue_sel.cpp \
# 	sue_sess.cpp sue_buf.cpp sue_inet.cpp sue_tcps.cpp sue_tcpc.cpp \
# 	sue_http.cpp sue_digs.cpp sue_wait.cpp \
# 	sue_smap.cpp sue_822.cpp sue_plst.cpp
# HEADERS = $(SOURCES:.cpp=.hpp)
# OBJECTS = $(SOURCES:.cpp=.o) md5.o

CSOURCES = sue_base.c sue_aloc.c sue_adfl.c sue_sigs.c
CXXSOURCES = 
HEADERS = $(CSOURCES:.c=.h) $(CXXSOURCES:.cpp=.hpp)
OBJECTS = $(CSOURCES:.c=.o) $(CXXSOURCES:.cpp=.o)


# for install.mk
LIBNAME = sue
LIBFILES = lib$(LIBNAME).a
HEADERFILES = $(HEADERS)	


default: libsue.a
	:


%.o:	%.cpp 
	$(CXX) $(CXXFLAGS) -c $<

%.o:	%.c
	$(CC) $(CFLAGS) -c $<

libsue.a:	$(OBJECTS)
	$(AR) crs $@ $^ 

tags:	$(CSOURCES) $(CXXSOURCES) $(HEADERS)
	$(CTAGS) $^

doxydocs:
	doxygen suedoxy.conf

clean:
	rm -f *.o *.bin *.a *~ buf deps.mk tags
	rm -rf __doxydocs
	cd demo && $(MAKE) clean

deps.mk: $(CSOURCES) $(CXXSOURCES) Makefile
	$(CC) -MM $(CSOURCES) > $@
	[ x != x"$(CXXSOURCES)" ] && $(CXX) -MM $(CXXSOURCES) >> $@ || true

ifneq (clean, $(MAKECMDGOALS))
ifneq (tags, $(MAKECMDGOALS))
-include deps.mk
endif
endif
