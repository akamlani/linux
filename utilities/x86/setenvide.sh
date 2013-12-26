#!/bin/sh

# usage 
usage()
{
     echo "Usage: $0 <top toolchain dir> <rootfs dir location>"
     echo "<Example Usage>"
     echo "Usage: $0 /home/user /mnt/svcfs"
}

if [ $# -ne 2 ]; then
     usage
     exit 1;
fi


# Set top level directory
# e.g. execute script as . setbuildenv.sh /mnt/svcfs 
SV_TOP_DIR=$1
SV_ROOTFS_DIR=$2

ARCH=ARM
CROSS_COMPILE=arm-none-linux-gnueabi-
CXX=${CROSS_COMPILE}gcc 
CC=${CROSS_COMPILE}gcc
LD=${CROSS_COMPILE}ld
STRIP=${CROSS_COMPILE}strip

SV_TOOLCHAIN_PATH=${SV_TOP_DIR}/toolchain/arm-2009q1-203/CodeSourcery/Sourcery_G++_Lite
SV_ROOTFS_USR_INC=${SV_ROOTFS_DIR}/usr/include
SV_ROOTFS_USR_LIB=${SV_ROOTFS_DIR}/usr/lib
SV_ROOTFS_LIB=${SV_ROOTFS_DIR}/lib
SV_IDE_ENV_PATH=$(pwd)

CPPFLAGS+="-I${SV_ROOTFS_USR_INC} -DCROSS_COMPILE_GCC"
LDFLAGS+="-L${SV_ROOTFS_LIB} -L${SV_ROOTFS_USR_LIB}"

PATH=${SV_TOOLCHAIN_PATH}/bin:$PATH

export CROSS_COMPILE
export CXX
export CC
export LD
export ARCH
export STRIP

export SV_TOOLCHAIN_PATH
export SV_IDE_ENV_PATH
export SV_ROOTFS_USR_INC
export SV_ROOTFS_USR_LIB
export SV_ROOTFS_LIB

export PATH

export CPPFLAGS
export LDFLAGS


