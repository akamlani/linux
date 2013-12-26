#!/bin/sh

# create cpio image followed by u-boot mkimage
cd rootfs-ramfs
find . | cpio -o -H newc | gzip > ../initramfs.cpio.gz
cd ..
mkimage -n 'Ramdisk Image'  -A arm -O linux -T ramdisk -a 0 -e 0 -C gzip -d initramfs.cpio.gz initramfs.ramdisk
echo "Done making Ramfs Image"
