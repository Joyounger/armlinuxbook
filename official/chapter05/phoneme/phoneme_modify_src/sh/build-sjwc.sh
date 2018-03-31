. setup.sh

make -C $HOME/midp/build/linux_qte_gcc \
GNU_TOOLS_DIR=$GNU_TOOLS_DIR \
JDK_DIR=$JDK_DIR \
PCSL_OUTPUT_DIR=$Output/pcsl \
CLDC_DIST_DIR=$Output/cldc/linux_arm/dist \
TOOLS_DIR=$HOME/tools \
TOOLS_OUTPUT_DIR=$Output/tools \
TARGET_CPU=arm \
MIDP_OUTPUT_DIR=$Output/midp/linux_qte_gcc \
$1

if [ $? -ne 0 ]; then
	echo "sjwc_status=failed" >> $Log
else
	echo "sjwc_status=OK" >> $Log
fi

. $Scripts/teardown.sh

