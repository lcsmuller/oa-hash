TOP = ..
CC = cc

OBJS = $(TOP)/oa_hash.o
EXES = test

CFLAGS += -Wall -Wextra -Wpedantic -g -I$(TOP) -std=c99

all: $(EXES)

$(EXES): $(OBJS)

clean:
	$(MAKE) -C $(TOP) $@
	rm -f $(EXES)

.PHONY : all clean
