#!/bin/sh

sudo chsh -s /bin/bash rukshani
sudo apt-get -y update
cd /opt/
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.1.tar.gz
tar -xf linux-5.15.1.tar.gz

export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true
sudo debconf-set-selections /opt/rotor-xdp/linux-setup/kexec-preseed.txt
sudo apt-get install -y build-essential libncurses-dev bison flex libssl-dev libelf-dev kexec-tools python3 python3-pip

pip install numpy pandas matplotlib cycler

echo "GRUB_DEFAULT=\"Advanced options for Ubuntu>Ubuntu, with Linux $(uname -r)\"" | sudo tee -a /etc/default/grub

cp /opt/rotor-xdp/linux-setup/linux_config /opt/linux-5.15.1/
mv /opt/linux-5.15.1/linux_config /opt/linux-5.5.1/.config

sudo apt-get install libdw-dev
cd /opt
git clone https://github.com/acmel/dwarves.git
cd /opt/dwarves/
git checkout tags/v1.22
sudo apt-get install cmake
sudo cmake -D__LIB=lib -DBUILD_SHARED_LIBS=OFF
sudo make install



