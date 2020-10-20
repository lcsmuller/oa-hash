CC = gcc
CFLAGS = -Wall -Werror -pedantic -g

.PHONY : clean

hashtable.o : hashtable.c
	$(CC) -c -fPIC $< -o $@ $(CFLAGS)

clean :
	rm -f hashtable.o

