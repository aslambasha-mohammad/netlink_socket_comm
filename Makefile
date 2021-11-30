BINARY = user
CC = gcc
RM = rm

SRCS = nl_user.c

OBJS = $(SRCS:.c=.o)

all: $(OBJS)
	@$(CC) -o $(BINARY) $(OBJS)
	@$(MAKE) -s build_module;

.c.o:
	@$(CC) .c $< -o $@

obj-m := nl_kernel.o

OS_NAME ?= $(shell uname -r)

KDIR = /lib/modules/$(OS_NAME)/build

EXTRA_CFLAGS = -I $(PWD)/

build_module:
	@make $(EXTRA_CFLAGS) -C $(KDIR) M=$(PWD) modules
	@echo "\033[1;32m" "[COMPILATION DONE]" "\033[m"

clean:
	@$(RM) $(BINARY) *.o
	@make -C $(KDIR) M=$(PWD) clean
