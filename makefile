OPT ?= -O3

LIBS := minivm
MINIVM := ./minivm

SRCS = src/seaweed.c src/strbuf.c
OBJS = $(SRCS:%.c=%.o)

default: all

all: bin/seaweed

bin/seaweed: $(OBJS) minivm/libminivm.a
	mkdir -p bin
	$(CC) $(OPT) $(OBJS) -o $(@) -L$(MINIVM) $(LIBS:%=-l%)

$(OBJS): $(@:%.o=%.c)
	$(CC) $(OPT) -c $(@:%.o=%.c) -o $(@) -I$(MINIVM)

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
