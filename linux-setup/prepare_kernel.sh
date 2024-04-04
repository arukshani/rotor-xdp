#!/bin/sh

sudo apt-get -y update
cd /opt/
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.1.tar.gz
tar -xf linux-5.15.1.tar.gz

export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true
sudo debconf-set-selections /opt/rotor-xdp/linux-setup/kexec-preseed.txt
sudo apt-get install -y build-essential libncurses-dev bison flex libssl-dev libelf-dev kexec-tools python3 python3-pip dwarves

pip install numpy pandas matplotlib cycler

echo "GRUB_DEFAULT=\"Advanced options for Ubuntu>Ubuntu, with Linux $(uname -r)\"" | sudo tee -a /etc/default/grub

cp /opt/rotor-xdp/linux-setup/linux_config /opt/linux-5.15.1/
mv /opt/linux-5.15.1/linux_config /opt/linux-5.15.1/.config



