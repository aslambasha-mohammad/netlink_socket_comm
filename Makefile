obj-m := nl_kernel.o

KDIR = /lib/modules/`uname -r`/build

EXTRA_CFLAGS = -I $(PWD)/

all:
	make $(EXTRA_CFLAGS) -C $(KDIR) M=`pwd` modules

clean:
	make -C $(KDIR) M=`pwd` clean
