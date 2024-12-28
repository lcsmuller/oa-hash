CC = cc

OBJS = oa_hash.o

CFLAGS += -Wall -Wextra -Wpedantic -std=c89

all: $(OBJS)

clean:
	rm -f $(OBJS)

.PHONY : all clean
