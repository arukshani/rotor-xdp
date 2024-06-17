#!/bin/sh

sudo ip netns exec ns1 bash

echo 300 | tee /proc/sys/net/ipv4/tcp_reordering
echo 1000 | tee /proc/sys/net/ipv4/tcp_max_reordering

echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_wmem
echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_rmem

exit
