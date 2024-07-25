#!/bin/sh

NODE_INTERFACE=$(ifconfig | grep -B1 "inet $1" | awk '$1!="inet" && $1!="--" {print $1}')
NODE_INTERFACE=${NODE_INTERFACE::-1}

# sudo ip link set $NODE_INTERFACE mtu 3490
# sudo ethtool -L $NODE_INTERFACE combined 1

sudo ethtool -G $NODE_INTERFACE rx 2048
sudo ethtool -G $NODE_INTERFACE tx 2048

# cd /opt/rotor-xdp/emulator-setup/
# sudo ./set_irq_affinity.sh $NODE_INTERFACE

# echo "00000020" | sudo tee /proc/irq/155/smp_affinity ##core5