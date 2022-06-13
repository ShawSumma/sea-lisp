OPT ?= -O3

LIBS := minivm
MINIVM := ./minivm

SRCS = src/seaweed.c src/strbuf.c src/parse.c src/ast.c src/lower.c
OBJS = $(SRCS:%.c=%.o)

default: all

all: bin/seaweed

bin/seaweed: $(OBJS) minivm/libminivm.a
	mkdir -p bin
	$(CC) $(OPT) $(LDFLAGS) $(OBJS) -o $(@) -L$(MINIVM) $(LIBS:%=-l%)

$(OBJS): $(@:%.o=%.c)
	$(CC) $(OPT) $(CFLAGS) -c $(@:%.o=%.c) -o $(@) -I$(MINIVM)

minivm/libminivm.a: minivm
	@echo "pushd ./ > /dev/null"
	@echo cd $(MINIVM) 
	@$(MAKE) --no-print-directory -C $(MINIVM) libminivm.a
	@echo "popd > /dev/null"

clean: .dummy
	rm -rf $(OBJS)
	@echo "pushd ./ > /dev/null"
	@echo cd $(MINIVM) 
	@$(MAKE) --no-print-directory -C $(MINIVM) clean
	@echo "popd > /dev/null"

.dummy:
