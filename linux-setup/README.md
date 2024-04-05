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

sudo apt install dwarves

pip install numpy pandas matplotlib cycler

# set default boot to current (vanila) kernel
echo "GRUB_DEFAULT=\"Advanced options for Ubuntu>Ubuntu, with Linux $(uname -r)\"" | sudo tee -a /etc/default/grub
```

```
cd /opt/linux-5.15.1/
cp /boot/config-`uname -r` .config
<!-- (//this should do the trick as well> scripts/config --set-str SYSTEM_TRUSTED_KEYS "") -->
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --set-str CONFIG_DEBUG_INFO_BTF "n"

Add the following in scripts/link-vmlinux.sh
if [ "${pahole_ver}" -ge "124" ]; then
    extra_paholeopt="${extra_paholeopt} --skip_encoding_btf_enum64"
fi 


<!-- make ARCH=x86 -->
make -j $(nproc)
sudo make modules_install
sudo make install

sudo kexec -l /boot/vmlinuz-5.15.1 --initrd=/boot/initrd.img-5.15.1 --reuse-cmdline
sudo systemctl kexec
```

```
make clean
rm .config
# optional: remove kernel soruce if they take too much space
# rm -rf *


```

## notes
```
BTF: .tmp_vmlinux.btf: pahole (pahole) is not available
Failed to generate BTF for vmlinux
Try to disable CONFIG_DEBUG_INFO_BTF
```

## loggin
```
CFLAGS_tcp_input.o := -DDEBUG (in tcp  modules's Makefile)
sudo dmesg -w
tail -F /var/log/messages

eg:
#if FASTRETRANS_DEBUG > 0
	if (ntohs(inet->inet_sport) == 4000)
	{
    	pr_debug("INSDIE tcp_ack pr_debug \n");
	}
#endif

or

printk(KERN_INFO "INSDIE tcp_ack from printk \n");

struct sock *sk;
struct inet_sock *inet  = inet_sk(sk); //RUK

	//RUK
	if (ntohs(inet->inet_sport) == 4000)
	{
    	printk(KERN_INFO "tcp_retransmit_timer \n");
	}

```
