#!/bin/sh

sudo pkill -9 nc
sudo pkill -9 tcpdump

if [ -f /var/run/tcpdump.pid ]
then
        sudo kill `cat /var/run/tcpdump.pid`
        sudo echo tcpdump `cat /var/run/tcpdump.pid` killed.
        sudo rm -f /var/run/tcpdump.pid
else
        echo tcpdump not running.
fi