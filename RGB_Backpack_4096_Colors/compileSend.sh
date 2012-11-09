#!/bin/bash
OMD5=""
while true
  do
    CMD5="$(md5sum RGB_Backpack_vFPO.c | gawk '{print $1}')" ; if [ "$CMD5" != "$OMD5" ] ; then make clean ; make && /home/fabio/Desktop/arduino-0018-ISP/hardware/tools/avrdude -C /home/fabio/Desktop/arduino-0018-ISP/hardware/tools/avrdude.conf -P /dev/ttyUSB* -b 19200 -c avrisp -p m328p -v -e -U flash:w:RGB_Backpack_vFPO.hex ; OMD5="$CMD5" ; fi ; done

