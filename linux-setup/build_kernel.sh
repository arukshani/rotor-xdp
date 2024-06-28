#!/bin/sh

cd /opt/linux-5.15.1/
make -j $(nproc)
sudo make modules_install
sudo make install
# sudo debconf-set-selections /opt/rotor-xdp/linux-setup/kexec-preseed.txt
# sudo kexec -l /boot/vmlinuz-5.15.1 --initrd=/boot/initrd.img-5.15.1 --reuse-cmdline
# sudo systemctl kexec

# make clean
# rm .config
# optional: remove kernel soruce if they take too much space
# rm -rf *

