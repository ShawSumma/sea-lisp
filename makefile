OPT ?= -O3

LIBS := minivm
MINIVM := ./minivm

SRCS = src/seaweed.c src/strbuf.c src/parse.c src/ast.c src/lower.c
OBJS = $(SRCS:%.c=%.o)

default: all

all: bin/seaweed.exe

bin/seaweed.exe: bin/seaweed
	cp bin/seaweed bin/seaweed.exe

bin/seaweed: $(OBJS) $(MINIVM)/bin/libminivm.a
	mkdir -p bin
	$(CC) $(OPT) $(LDFLAGS) $(OBJS) -o $(@) $(MINIVM)/bin/libminivm.a

$(OBJS): $(@:%.o=%.c)
	$(CC) $(OPT) $(CFLAGS) -c $(@:%.o=%.c) -o $(@)

$(MINIVM)/bin/libminivm.a: minivm
	$(MAKE) --no-print-directory -C $(MINIVM) bin/libminivm.a

clean: .dummy
	rm -rf $(OBJS)
	$(MAKE) --no-print-directory -C $(MINIVM) clean

.dummy:
