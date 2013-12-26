#!/bin/sh

mkfs.ubifs -v -r rootfs -o rootfs.ubifs -m 2048 -e 129024 -c 1996
ubinize -v -o rootfs.ubi -m 2048 -p 128KiB -s 512 ubinize.cfg

echo "Rootfs input folder: rootfs"
echo "Done Making Rootfs: Output: rootfs.ubi"
