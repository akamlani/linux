# Pre-Conditions
# Configure the environment variable (1-5) setenv nandburner_option <<option>>

# Basic Environment Variables
MLO=1
UBOOT=2
UIMAGE=3
ROOTFS=4
LOADER=5
ALL=6
ENV_VAR=7

SUCCESS="GOOD"
FAIL="BAD"

# Memory Map Environment Variables
MTD0_START=000000000000
MTD0_END=000000080000
MTD1_END=000000240000
MTD2_END=000000280000
MTD3_END=000000680000
MTD4_END=00001F600000
MTD5_END=00001F880000
MTD6_END=000020000000

# Initialize
state=${FAIL}
nandburner_option=666

mlo_state=${FAIL}
uboot_state=${FAIL}
uimage_state=${FAIL}
rootfs_state=${FAIL}
loader_kernel_state=${FAIL} 
loader_ramfs_state=${FAIL}
envvar_state=${FAIL}

# Crude Menu
echo Set the following numerical option in the environment variable (nandburner_option)
echo ${MLO}: MLO
echo ${UBOOT}: uboot
echo ${UIMAGE}: uImage
echo ${ROOTFS}: rootFS
echo ${LOADER}: update loader
echo ${ALL}: all binaries
echo ${ENV_VAR}: environment variables


# Set environment
echo
echo Chosen Value: ${nandburner_option}
echo


# Based on the following MTD Partitions
# verify with cat /proc/mtd on device
# change environment variables above accordingly
# 7 MTD Partitions on "omap2-nand.0" (MTD0-MTD6)
# Current Partion Table
echo  =====================================================
echo  NAND Partition Table
echo  0x000000000000-0x000000080000 : "xloader"
echo  0x000000080000-0x000000240000 : "uboot"
echo  0x000000240000-0x000000280000 : "uboot environment"
echo  0x000000280000-0x000000680000 : "linux"
echo  0x000000680000-0x00001F600000 : "rootfs"
echo  0x00001f600000-0x00001f880000 : "update loader kernel"
echo  0x00001f880000-0x000020000000 : "update loader ramfs"
echo  =====================================================
echo

# Handle options
# Reflash MLO (MTD0) to each of first 4 blocks in NAND
# Changed for max sizes so MLO won't have trouble with page size alignment
# Page Size alignment has trouble with some older versions of u-boot (e.g. 2010-09)
if test ${nandburner_option} -eq ${MLO} -o ${nandburner_option} -eq ${ALL} 
then 
   echo CLEAR AND WRITE x-loader
   echo 0x${MTD0_START} 0x${MTD0_END}
   mmc rescan 0
   if fatload mmc 0 ${loadaddr} MLO; then
      nandecc hw
      nand erase ${MTD0_START} ${MTD0_END}
      nand write ${loadaddr} 	0 	20000
      nand write ${loadaddr} 	20000	20000
      nand write ${loadaddr} 	40000	20000
      nand write ${loadaddr}	60000	20000
      state=${SUCCESS}
      mlo_state=${SUCCESS}
   fi
fi

# Reflash UBOOT (MTD1)
# CLEAR UBOOT Settings Area (MTD2)
# CLEAR UBOOT settings anytime we muck with UBOOT
# Max size for u-boot write (padded) due to ECC Failures on later u-boot
if test ${nandburner_option} -eq ${UBOOT} -o ${nandburner_option} -eq ${ALL} 
then 
   echo CLEAR AND WRITE UBOOT
   echo 0x${MTD0_END} 0x${MTD1_END}
   mmc rescan 0
   if fatload mmc 0 ${loadaddr} u-boot.bin; then
      nandecc sw
      nand erase ${MTD0_END} 1c0000
      nand write ${loadaddr} ${MTD0_END} $filesize
      state=${SUCCESS}
      uboot_state=${SUCCESS}
   fi

   echo CLEAR UBOOT ENVIRONMENT SETTINGS
   echo 0x${MTD1_END} 0x${MTD2_END}
   nand erase ${MTD1_END} 20000
fi

# Reflash Kernel uImage
if test ${nandburner_option} -eq ${UIMAGE} -o ${nandburner_option} -eq ${ALL} 
then 
   echo CLEAR AND WRITE UIMAGE
   echo 0x${MTD2_END} 0x${MTD3_END}
   mmc rescan 0
   if fatload mmc 0 ${loadaddr} uImage; then
      nandecc sw
      nand erase ${MTD2_END} 00400000
      nand write ${loadaddr} ${MTD2_END} ${filesize}
      state=${SUCCESS}
      uimage_state=${SUCCESS}
   fi
fi


# Reflash Rootfs (MTD4) - (UBIFS successor to JFFS2)
if test ${nandburner_option} -eq ${ROOTFS} -o ${nandburner_option} -eq ${ALL} 
then 
   echo CLEAR AND WRITE ROOTFS
   echo 0x${MTD3_END} 0x${MTD4_END}
   mmc rescan 0
   if fatload mmc 0 ${loadaddr} rootfs.ubi; then 
      nandecc sw
      nand erase ${MTD3_END} 1EF80000
      nand write ${loadaddr} ${MTD3_END} ${filesize}
      state=${SUCCESS}
      rootfs_state=${SUCCESS}
   fi
fi

# Reflash Update Loader (MTD5)/(MTD6)
if test ${nandburner_option} -eq ${LOADER} -o ${nandburner_option} -eq ${ALL}
then
   echo CLEAR and WRITE UPDATE LOADER {KERNEL, RAMFS}
   echo 0x${MTD4_END} 0x${MTD6_END}
   mmc rescan 0
   if fatload mmc 0 ${loadaddr} update-loader-image; then
      nandecc sw
      nand erase ${MTD4_END} 280000
      nand write ${loadaddr} ${MTD4_END} ${filesize}
      loader_kernel_state=${SUCCESS}
   fi
   if fatload mmc 0 ${loadaddr} initramfs.ramdisk; then
      nandecc sw
      nand erase ${MTD5_END} 780000
      nand write ${loadaddr} ${MTD5_END} ${filesize}
      loader_ramfs_state=${SUCCESS}
   fi
   if test "${loader_kernel_state}" = "${SUCCESS}" -a "${loader_ramfs_state}" = "${SUCCESS}"; then
      state=${SUCCESS}
   fi
fi

# Set all environment variables
if test ${nandburner_option} -eq ${ENV_VAR}
then 
   echo Configure Rootfs Env Variables 
   nandecc sw
   printenv nandrootfstype
   printenv nandroot
   printenv console
   printenv optargs
   printenv bootdelay
   printenv mmcargs
   printenv nandargs

   setenv nandroot ubi0:rootfs ubi.mtd=4 ro
   setenv nandrootfstype ubifs
   setenv console ttyS2,115200n8
   setenv optargs quiet lpj=1949696
   setenv bootdelay 0
   setenv mmcargs $mmcargs \${optargs}
   setenv nandargs $nandargs \${optargs}
   saveenv

   printenv nandrootfstype
   printenv nandroot
   printenv console
   printenv optargs
   printenv bootdelay
   printenv mmcargs
   printenv nandargs

   state=${SUCCESS}
   envvar_state=${SUCCESS}
fi

# Test Final Status programming all binaries
if test ${nandburner_option} -eq ${ALL}; then
   state=${FAIL}
   if test "${mlo_state}" = "${SUCCESS}" -a "${uboot_state}" = "${SUCCESS}"; then
      if test "${uimage_state}" = "${SUCCESS}" -a "${rootfs_state}" = "${SUCCESS}"; then 
         if test "${loader_kernel_state}" = "${SUCCESS}" -a "${loader_ramfs_state}" = "${SUCCESS}"; then  
	    state=${SUCCESS} 
         fi
      fi
   fi
fi


# Log Completion
if test ${state} = ${SUCCESS};
then
   echo ===================================================== 
   echo FLASH PROGRAMMING COMPLETE!!!
   echo ===================================================== 
else
   echo
   echo FAILURE -TRY AGAIN HOMMIE!!!
   echo ===================================================== 
fi


# Make the Scripts Executable by U-Boot
# mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "nand-burner" -d nand-burner.cmd nand-burner.scr
# At prompt (assuming flash-all name os script created)
# mmc rescan 0; fatload mmc 0 ${loadaddr} nand-burner.scr; source ${loadaddr}
