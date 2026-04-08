# This Makefile was added by Andrey "Croco" Stolyarov
# for the FedaNet project.  Please note it is only
# needed for unified library handling, but certainly
# it's kinda overkill to have a Makefile and specially
# a libXX.a file for a ``library'' that consists of
# one module.
#
# If you don't like this, simply download the original
# monocypher tarball and enjoy :-)
#


CC=gcc
CFLAGS=-std=c99 -Wall -g -O3

# we tolerate c99 as of now, but this is to be fixed

NAME=monocypher

lib$(NAME).a: $(NAME).o
	ar -rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o lib$(NAME).a
