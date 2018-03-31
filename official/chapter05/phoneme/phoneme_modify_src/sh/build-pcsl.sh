. setup.sh

make -C $HOME/pcsl \
GNU_TOOLS_DIR=$GNU_TOOLS_DIR \
PCSL_PLATFORM=linux_arm_gcc \
PCSL_OUTPUT_DIR=$Output/pcsl \
NETWORK_MODULE=bsd/qte \
TOOLS_DIR=$HOME/tools \
TOOLS_OUTPUT_DIR=$Output/tools \
$1

if [ $? -ne 0 ]; then
	echo "build-pcsl=failed" >> $Log
else
	echo "build-pcsl=OK" >> $Log
fi

. $Scripts/teardown.sh

