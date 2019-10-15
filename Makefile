ifneq  ($(KERNELRELEASE),)
obj-m:=test4_kernel.o
else
KDIR := /usr/src/kernels/2.6.32-754.23.1.el6.x86_64/
PWD:=$(shell pwd)
all:
    make -C $(KDIR) M=$(PWD) modules
clean:
    rm -f *.ko *.o *.symvers *.cmd *.cmd.o
endif