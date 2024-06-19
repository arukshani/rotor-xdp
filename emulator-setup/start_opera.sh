#!/bin/sh

# echo $1

# sudo ./opera_nic 192.168.1.1 configs/node-1-link.csv 60
echo $1
echo $2
echo $3
echo $4
echo $5
cd /opt/rotor-xdp/$4
nohup sudo ./emu_nic $1 config/$2.csv /dev/$3 $5 1 1 > /tmp/start_log 2>&1 &
# nohup sudo ./emu_nic $1 ptp-32node-test-config/$2.csv /dev/$3 240 1 1 > /tmp/start_log 2>&1 &
echo "======================================================================="
# nohup sudo ./opera_nic $1 configs/$2.csv $3 120 > /tmp/opera_nic.out 2>&1 &