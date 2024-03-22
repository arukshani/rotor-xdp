#!/bin/sh

# echo $1

# sudo ./opera_nic 192.168.1.1 configs/node-1-link.csv 60
cd /opt/rotor-xdp/emu-opera-final
nohup sudo ./emu_nic $1 config/$2.csv /dev/$3 60 1 1 > /dev/null 2>&1 &
# nohup sudo ./opera_nic $1 configs/$2.csv $3 120 > /tmp/opera_nic.out 2>&1 &