obj-m := nl_kernel.o

OS_NAME ?= $(shell uname -r)

KDIR = /lib/modules/$(OS_NAME)/build

EXTRA_CFLAGS = -I $(PWD)/

all:
	make $(EXTRA_CFLAGS) -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
