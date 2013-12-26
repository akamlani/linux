#!/bin/sh

# Set top level directory

usage()
{
     echo "Usage: $0 <top dir>"
     echo "<Example Usage>"
     echo "Usage: $0 /home/user"
}

if [ $# -ne 1 ]; then
     usage
     exit 1;
fi


SV_TOP_DIR=$1

ARCH=arm
CROSS_COMPILE=arm-none-linux-gnueabi-
PATH=${SV_TOP_DIR}/toolchain/arm-2009q1-203/CodeSourcery/Sourcery_G++_Lite/bin:$PATH
PATH=${SV_TOP_DIR}/build/src/u-boot/tools:$PATH

SV_BUILD_PATH=${SV_TOP_DIR}/build
SV_STAGING_PATH=${SV_BUILD_PATH}/staging/images
SV_TOOLCHAIN_PATH=${SV_TOP_DIR}/toolchain/arm-2009q1-203/CodeSourcery/Sourcery_G++_Lite

SV_KERNEL_RAMFS_PATH=${SV_BUILD_PATH}/src/kernel/linux-omap-2.6-ramfs
SV_KERNEL_PATH=${SV_BUILD_PATH}/src/kernel/linux-omap-2.6
SV_KERNEL_HDR_PATH=${SV_KERNEL_PATH}/usr/include

SV_SVN_GUMSTIX=${SV_TOP_DIR}/svn/projects/VectorII/Gumstix

export CROSS_COMPILE
export ARCH
export PATH

export SV_BUILD_PATH
export SV_STAGING_PATH
export SV_TOOLCHAIN_PATH

export SV_KERNEL_RAMFS_PATH
export SV_KERNEL_PATH
export SV_KERNEL_HDR_PATH

export SV_SVN_GUMSTIX
