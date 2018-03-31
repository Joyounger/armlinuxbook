#!/bin/sh

make clean
make
cp demo_console /home/lipeng/nfsroot/rootfs-2.6/ -f
cp demo_console /tftpboot/lipeng/ -f
cp demo_fbtest /tftpboot/lipeng/ -f

