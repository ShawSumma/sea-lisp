OPT ?= -O3

MINIVM := ./minivm

SRCS = src/seaweed.c src/strbuf.c src/parse.c src/ast.c src/lower.c
OBJS = $(SRCS:%.c=%.o)

default: all

all: bin/seaweed

bin/seaweed: $(OBJS) minivm/bin/libminivm.a
	mkdir -p bin
	$(CC) $(OPT) $(LDFLAGS) $(OBJS) -o $(@) minivm/bin/libminivm.a

$(OBJS): $(@:%.o=%.c)
	$(CC) $(OPT) $(CFLAGS) -c $(@:%.o=%.c) -o $(@) -I$(MINIVM)

minivm/bin/libminivm.a: minivm
	$(MAKE) --no-print-directory -C $(MINIVM) bin/libminivm.a

clean: .dummy
	rm -rf $(OBJS)
	$(MAKE) --no-print-directory -C $(MINIVM) clean

.dummy:
