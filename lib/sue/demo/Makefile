CC = gcc
CFLAGS = -g -Wall -O0 -I../..
CXX = g++
CXXFLAGS = -g -Wall -O0 -I../.. 

PROGS = b_sleep5 b_hooks b_sitter b_sigs

all:	$(PROGS)

../libsue.a:	FORCE	
	cd .. && $(MAKE) libsue.a

%:	%.cpp ../libsue.a
	$(CXX) $(CXXFLAGS) $< ../libsue.a -o $@

%:	%.c ../libsue.a
	$(CC) $(CFLAGS) $< ../libsue.a -o $@

clean:
	rm -f *.o *~ deps.mk $(PROGS)

FORCE:
