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

### PTP start and stop and remove telemetry logs
```
python3 ptp_script.py -s 
python3 ptp_script.py -k 
python3 ptp_script.py -r
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

//minimum, default and maximum
net.ipv4.tcp_rmem=4096 65536 1073741824 
net.ipv4.tcp_wmem=4096 65536 1073741824

echo 300 | tee /proc/sys/net/ipv4/tcp_reordering
echo 1000 | tee /proc/sys/net/ipv4/tcp_max_reordering

disable RACK (inside namespace)
echo 0 | tee /proc/sys/net/ipv4/tcp_recovery

echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_wmem
echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_rmem

https://fasterdata.es.net/host-tuning/linux/
(67108864 bytes = 67MB ; 67108864/3400=19737 packets)
echo net.ipv4.tcp_wmem = 4096 67108864 1073741824 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 67108864 1073741824 | tee -a /etc/sysctl.conf
sysctl -p

sysctl net.ipv4.tcp_reordering
sysctl net.ipv4.tcp_max_reordering
sysctl net.ipv4.tcp_wmem
sysctl net.ipv4.tcp_rmem
```

### Partial node setup
python3 partial_node_setup.py
python3 partial_setup_arp.py

echo 8 | sudo tee /proc/sys/kernel/printk

ifconfig down/ifconfig up

ifconfig vethin2 up
ifconfig vethin2 down
