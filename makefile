OPT ?= -O3

LIBS := minivm
MINIVM := ./minivm

SRCS = src/seaweed.c src/strbuf.c src/parse.c src/ast.c src/lower.c
OBJS = $(SRCS:%.c=%.o)

default: all

all: bin/seaweed

bin/seaweed: $(OBJS) $(MINIVM)/bin/libminivm.lib
	mkdir -p bin
	$(CC) $(OPT) $(LDFLAGS) $(OBJS) -o $(@) $(MINIVM)/bin/libminivm.lib

$(OBJS): $(@:%.o=%.c)
	$(CC) $(OPT) $(CFLAGS) -c $(@:%.o=%.c) -o $(@)

$(MINIVM)/bin/libminivm.lib: minivm
	$(MAKE) --no-print-directory -C $(MINIVM) bin/libminivm.lib

$(MINIVM)/bin/libminivm.a: minivm
	$(MAKE) --no-print-directory -C $(MINIVM) bin/libminivm.a

clean: .dummy
	rm -rf $(OBJS)
	$(MAKE) --no-print-directory -C $(MINIVM) clean

.dummy:
