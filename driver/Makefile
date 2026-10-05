obj-m += sysguard.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
ccflags-y := -I$(PWD)/../include -Wall

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.o *.ko *.mod.* *.order *.symvers .*.cmd
