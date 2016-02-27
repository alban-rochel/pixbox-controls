#!/bin/bash

dtc -O dtb -o PixBox-00A0.dtbo -b 0 -@ PixBox.dts
cp PixBox-00A0.dtbo /lib/firmware/

