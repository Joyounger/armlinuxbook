mount -t nfs 192.168.0.10:/mnt/nfs /mnt/nfs -o nolock,rsize=1024,wsize=1024,timeo=15
export PATH=/mnt/nfs/image/bin:$PATH
export LD_LIBRARY_PATH=/mnt/nfs/image/lib:$LD_LIBRARY_PATH
export QWS_MOUSE_PROTO="MouseMan:/dev/input/mice IntelliMouse:/dev/misc/psaux"
export QWS_KEYBOARD="matrixkbd:/dev/mcu/kbd TTY:/dev/vc/0"
export QWS_DISPLAY="LinuxFb:mmWidth85:mmHeight48:0"
