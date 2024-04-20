#!/bin/sh

sudo apt-get -y update
cd /opt
git clone https://github.com/arukshani/rotor-xdp.git 
sudo apt-get -y install clang llvm libelf-dev libpcap-dev gcc-multilib build-essential
sudo apt-get -y install linux-tools-$(uname -r)
sudo apt-get -y install linux-headers-$(uname -r)
sudo apt-get -y install linux-tools-common linux-tools-generic
sudo apt-get -y install tcpdump
sudo apt-get -y install jq
sudo apt-get -y install linuxptp
sudo apt-get -y install libmnl-dev
sudo apt-get -y install m4
sudo apt-get -y install iperf3
sudo apt-get -y install htop

#Get interface 
NODE_INTERFACE=$(ifconfig | grep -B1 "inet $1" | awk '$1!="inet" && $1!="--" {print $1}')
NODE_INTERFACE=${NODE_INTERFACE::-1}

# echo $NODE_INTERFACE

echo 2| sudo tee /sys/class/net/$NODE_INTERFACE/napi_defer_hard_irqs
echo 1000 | sudo tee /sys/class/net/$NODE_INTERFACE/gro_flush_timeout
# echo 200000 | sudo tee /sys/class/net/$NODE_INTERFACE/gro_flush_timeout


