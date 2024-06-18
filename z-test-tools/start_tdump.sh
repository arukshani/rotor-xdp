#!/bin/sh

NODE_IN=$(ifconfig | grep -B1 "inet $1" | awk '$1!="inet" && $1!="--" {print $1}')
echo $NODE_IN
NODE_IN=${NODE_IN::-1}

pcap_name="host-tcp-dump.pcap"
echo $pcap_name

rm -rf /tmp/$pcap_name
# sudo nohup tcpdump -i $NODE_IN -j adapter_unsynced -w /tmp/$pcap_name > /dev/null 2>&1 &
nohup tcpdump -i $NODE_IN -w /tmp/$pcap_name > /dev/null 2>&1 &
echo $! | sudo tee /var/run/tcpdump.pid