#!/bin/sh

make clean
make
cp demo_console.out /home/lipeng/nfsroot/rootfs-2.6/ -f
cp demo_console.out /tftpboot/lipeng/ -f

cp fbtest.out /home/lipeng/nfsroot/rootfs-2.6/ -f
cp fbtest.out /tftpboot/lipeng/ -f
