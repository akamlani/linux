#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "rover_drv_gpio.h"

int main()
{
   int fd  = -1;
   int err = -1;
   fd = open( ROVER_DRV_DEV_NAME, O_RDONLY );
/*
   //Test1: Blocking ioctl
   printf("Test1-A ROVER_GPIO_PEND_INTR\n");
   err = ioctl(fd, ROVER_GPIO_PEND_INTR);
   printf("Test1-B ROVER_GPIO_PEND_INTR Err:%d\n", err);

   //Test2: Simulate intr
   printf("Test2-A ROVER_GPIO_PERFORM_INTR\n");
   err = ioctl(fd, ROVER_GPIO_PERFORM_INTR);
   printf("Test2-B ROVER_GPIO_PERFORM_INTR Err:%d\n", err);

   //Test3: Blocking ioctl
   printf("Test3-A ROVER_GPIO_PEND_INTR\n");
   err = ioctl(fd, ROVER_GPIO_PEND_INTR);
   printf("Test3-B ROVER_GPIO_PEND_INTR Err:%d\n", err);
*/

   //Test0: Dump intr status information
   printf("Test0-A ROVER_GPIO_DUMP_INFO\n");
   err = ioctl(fd, ROVER_GPIO_DUMP_INFO);
   printf("Test0-B ROVER_GPIO_DUMP_INFO Err:%d\n", err);

   close(fd);
   fd = -1;

   return 0;
}
