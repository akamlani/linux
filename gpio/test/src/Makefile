# dedicated driver test for GPIO Driver
#

APPNAME		= rover-drv-status
OBJ_FILES       = rover_drv_test.o

CFLAGS		= -I../../driver

PWD       	= $(shell pwd)
CROSS_COMPILE   = arm-none-linux-gnueabi-
CC		= $(CROSS_COMPILE)gcc
ARCH		= arm


$(APPNAME) : $(OBJ_FILES)
	@echo "Creating executable $@ ..."
	$(CC) -o $(APPNAME) $(OBJ_FILES)

clean:
	rm -rf $(APPNAME) *.o

strip:
	$(STRIP) $(APPNAME)
