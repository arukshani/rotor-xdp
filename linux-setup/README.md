```
scp ~/.ssh/rukshani_cloudlab.pem rukshani@ip:~/.ssh/
chown -R rukshani /opt
cd /opt
git clone https://github.com/arukshani/rotor-xdp.git
cd /opt/
```





```
cd /opt/
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.1.tar.gz
tar -xf linux-5.15.1.tar.gz
```

```
sudo apt-get -y update
sudo chsh -s /bin/bash rukshani
exit
export DEBIAN_FRONTEND=noninteractive DEBCONF_NONINTERACTIVE_SEEN=true
sudo debconf-set-selections /opt/rotor-xdp/linux-setup/kexec-preseed.txt
sudo apt-get install -y build-essential libncurses-dev bison flex libssl-dev libelf-dev kexec-tools python3 python3-pip

pip install numpy pandas matplotlib cycler

# set default boot to current (vanila) kernel
echo "GRUB_DEFAULT=\"Advanced options for Ubuntu>Ubuntu, with Linux $(uname -r)\"" | sudo tee -a /etc/default/grub
```

```
cd /opt/linux-5.15.1/
cp /boot/config-`uname -r` .config
(//this should do the trick as well> scripts/config --set-str SYSTEM_TRUSTED_KEYS "")
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
make ARCH=x86
make -j $(nproc)
sudo make modules_install
sudo make install
```

```
make clean
rm .config
# optional: remove kernel soruce if they take too much space
# rm -rf *
```