#!/bin/sh

NODE_INTERFACE=$(ifconfig | grep -B1 "inet $1" | awk '$1!="inet" && $1!="--" {print $1}')
NODE_INTERFACE=${NODE_INTERFACE::-1}

sudo ip link set $NODE_INTERFACE mtu 3490
sudo ethtool -L $NODE_INTERFACE combined 1