CC = gcc
CFLAGS = -Wall -Werror -pedantic -g -fPIC

.PHONY : clean

hashtable.o : hashtable.c

clean :
	rm -f hashtable.o

