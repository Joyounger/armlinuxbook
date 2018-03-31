 ifeq ($(KERNELRELEASE),)
#KERNELDIR ?= /source/kernel/linux-2.6.8.1-farsight
KERNELDIR ?= /root/dev_home/kernel/linux-2.6.19
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
.PHONY:
	modules modules_install clean
else
	obj-m := s3c2410-adc.o
endif
