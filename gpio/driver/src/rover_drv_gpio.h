#pragma once
/****************************************************************************
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
****************************************************************************
*
*   Interface file for gpio function driver, 
*   allows user-mode access to control rover functionality (e.g. GPIO).
*
*****************************************************************************/


#include <linux/ioctl.h>

#define ROVER_DRV_NAME 			"rover-drv"
#define ROVER_DRV_DEV_NAME		"/dev/rover-drv"

#define IOC_ROVERDEV_MAGIC 'R'
#define ROVER_GPIO_PEND_INTR		_IO(IOC_ROVERDEV_MAGIC, 10)	//wait on pending interrupt
#define ROVER_GPIO_PERFORM_INTR		_IO(IOC_ROVERDEV_MAGIC, 11)	//simulate pending interrupt
#define ROVER_GPIO_DUMP_INFO		_IO(IOC_ROVERDEV_MAGIC, 12)	//dump driver useful info


#define EROVER_DRV_BASE			100
#define EROVER_GPIO_INVALID		-(EROVER_DRV_BASE + 1)
#define EROVER_GPIO_REQUEST		-(EROVER_DRV_BASE + 2)		
#define EROVER_GPIO_SETUP		-(EROVER_DRV_BASE + 3)
#define EROVER_INTR_ACCESS		-(EROVER_DRV_BASE + 4)	
#define EROVER_CREATE_CLASS_DEV		-(EROVER_DRV_BASE + 5)

