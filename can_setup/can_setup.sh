#!/vendor/bin/sh

export PATH=/system/bin:/vendor/bin:$PATH
 
ip link set can0 down
ip link set can0 type can bitrate 500000
ip link set can0 up
