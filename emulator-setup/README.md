### Master Node (Last node of the cluster)

```
scp ~/.ssh/rukshani_cloudlab.pem rukshani@ip:~/.ssh/
chown -R rukshani /opt
cd /opt
git clone https://github.com/arukshani/rotor-xdp.git
cd /opt/rotor-xdp/emulator-setup
./setup_master.sh
python3 setup_cloudlab.py
python3 setup_arp.py
```

### if you want to delete all arp records
```
for e in $(arp -a | sed -n 's/.*(\([^()]*\)).*/\1/p'); do arp -d $e; done
```

### ssh into namespace
```
sudo ip netns exec ns1 bash
sudo ip netns exec ns2 bash
```

### PTP start and stop
```
python3 ptp_script.py -s 
python3 ptp_script.py -k 
```

### Make, Clean, Pull and Start Opera Code
```
python3 opera_build.py -p //pull
python3 opera_build.py -m //make
python3 opera_build.py -c //clean
python3 opera_build.py -s //start
```

### Tunning
```

net.ipv4.tcp_rmem=4096 65536 1073741824 
net.ipv4.tcp_wmem=4096 65536 1073741824

echo 3 | tee /proc/sys/net/ipv4/tcp_reordering
echo 300 | tee /proc/sys/net/ipv4/tcp_max_reordering

https://fasterdata.es.net/host-tuning/linux/
echo net.ipv4.tcp_wmem = 4096 67108864 67108864 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 67108864 67108864 | tee -a /etc/sysctl.conf
sysctl -p

sysctl net.ipv4.tcp_reordering
sysctl net.ipv4.tcp_max_reordering
sysctl net.ipv4.tcp_wmem
sysctl net.ipv4.tcp_rmem
```


