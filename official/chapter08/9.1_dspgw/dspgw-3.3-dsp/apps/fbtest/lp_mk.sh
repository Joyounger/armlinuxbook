#!/bin/sh

make clean
make

cp fbtest.out /home/lipeng/nfsroot/rootfs-2.6/ -f
cp fbtest.out /tftpboot/lipeng/ -f
