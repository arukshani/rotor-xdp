#!/bin/sh

NODE_IN=$(ifconfig | grep -B1 "inet $1" | awk '$1!="inet" && $1!="--" {print $1}')
NODE_IN=${NODE_IN::-1}

pcap_name="host-br_time.pcap"
echo $pcap_name

sudo rm -rf /tmp/$pcap_name
sudo nohup tcpdump -i $NODE_IN -j adapter_unsynced -w /tmp/$pcap_name > /dev/null 2>&1 &
echo $! | sudo tee /var/run/tcpdump.pid