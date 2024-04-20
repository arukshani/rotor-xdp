#!/bin/sh

for i in $(seq 2 32); do 
    ns_ip_addr="192.168.$i.2"
    # echo $ns_ip_addr
    sudo ip netns exec ns1 ping $ns_ip_addr -c 5
done 