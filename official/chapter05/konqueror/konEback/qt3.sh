#!/bin/sh
export QTDIR=/arm2410s/gui/Qt/src/qt-2.3.10
export QPEDIR=/arm2410s/gui/Qt/src/qtopia-free-2.1.1
export TMAKEDIR=/arm2410s/gui/Qt/src/tmake-1.13
export TMAKEPATH=$TMAKEDIR/lib/qws/linux-arm-g++
export PATH=$QTDIR/bin:$QPEDIR/bin:$TMAKEDIR/bin:$PATH
export AR=arm-linux-ar
export STRIP=arm-linux-strip
export RANLIB=arm-linux-ranlib
export CXX=arm-linux-g++
export CCC=arm-linux-c++
export CC=arm-linux-gcc
export CROSS_COMPILE=1
export PATH=$PATH:/usr/local/arm/3.4.1/arm-linux/bin:$PATH
export INSTALL=/usr/local/arm/3.4.1/arm-linux
