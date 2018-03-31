. setup.sh

make -C $HOME/cldc/build/linux_arm \
GNU_TOOLS_DIR=$GNU_TOOLS_DIR \
JDK_DIR=$JDK_DIR \
ENABLE_PCSL=true \
PCSL_OUTPUT_DIR=$Output/pcsl \
JVMWorkSpace=$HOME/cldc \
JVMBuildSpace=$Output/cldc \
TOOLS_DIR=$HOME/tools \
TOOLS_OUTPUT_DIR=$Output/tools \
$1

if [ $? -ne 0 ]; then
	echo "cldc_status=failed" >> $Log
else
	echo "cldc_status=OK" >> $Log
fi

. $Scripts/teardown.sh

