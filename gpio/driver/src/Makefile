# dedicated driver for GPIO Driver
#

obj-m		:= rover_drv.o


PWD       	= $(shell pwd)
CROSS_COMPILE   = arm-none-linux-gnueabi-
CC		= ${CROSS_COMPILE}gcc
KERNEL_PATH     = ${SV_KERNEL_PATH}
ARCH		= arm

export ARCH CROSS_COMPILE CC

KBUILD_FLAGS 	= -C $(KERNEL_PATH) M=$(PWD)

default: modules

modules:
	echo $(MAKE) $(KBUILD_FLAGS) modules
	$(MAKE) $(KBUILD_FLAGS) modules

clean:
	rm -rf *.o *~ *.ko *.mod.c .*.cmd .tmp* modules.order Module.symvers
