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
python3 opera_build.py opera-v2 120 -p //pull
python3 opera_build.py opera-v2 120 -m //make
python3 opera_build.py opera-v2 120 -c //clean
python3 opera_build.py opera-v2 200 -s //start
python3 opera_build.py opera-v2 120 -k //kill
```

### Default
```
net.ipv4.tcp_wmem = 4096   16384   4194304
net.ipv4.tcp_rmem = 4096   131072  6291456

sysctl -w net.ipv4.tcp_rmem=4096 16384 4194304
sysctl -w net.ipv4.tcp_wmem=4096 16384 4194304

echo 3 | tee /proc/sys/net/ipv4/tcp_reordering
echo 100 | tee /proc/sys/net/ipv4/tcp_max_reordering
echo 1 | tee /proc/sys/net/ipv4/tcp_recovery
sysctl -p
```

### Tunning
```
echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_wmem
echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_rmem
echo 3000 | tee /proc/sys/net/ipv4/tcp_reordering
echo 5000 | tee /proc/sys/net/ipv4/tcp_max_reordering
echo 0 | tee /proc/sys/net/ipv4/tcp_recovery
sysctl -p

//minimum, default and maximum
net.ipv4.tcp_rmem=4096 65536 1073741824 
net.ipv4.tcp_wmem=4096 65536 1073741824

echo 300 | tee /proc/sys/net/ipv4/tcp_reordering
echo 1000 | tee /proc/sys/net/ipv4/tcp_max_reordering

disable and enable RACK (inside namespace)
echo 0 | tee /proc/sys/net/ipv4/tcp_recovery
echo 1 | tee /proc/sys/net/ipv4/tcp_recovery

echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_wmem
echo 4096 67108864 1073741824 | tee /proc/sys/net/ipv4/tcp_rmem

sudo ethtool -G enp65s0f0np0 rx 2048
sudo ethtool -G enp65s0f0np0 tx 2048


https://fasterdata.es.net/host-tuning/linux/
(67108864 bytes = 67MB ; 67108864/3400=19737 packets)
echo net.ipv4.tcp_wmem = 4096 67108864 1073741824 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 67108864 1073741824 | tee -a /etc/sysctl.conf

### Following doesn't cause packet drops in controller
echo net.ipv4.tcp_wmem = 4096 65536 6291456 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 65536 6291456 | tee -a /etc/sysctl.conf

echo net.ipv4.tcp_wmem = 4096 16384 4194304 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 131072 6291456 | tee -a /etc/sysctl.conf

## For paper
echo net.ipv4.tcp_wmem = 4096 16384 4194304 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 131072 6291456 | tee -a /etc/sysctl.conf

echo net.ipv4.tcp_rmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf

sysctl -p

sysctl net.ipv4.tcp_reordering
sysctl net.ipv4.tcp_max_reordering
sysctl net.ipv4.tcp_wmem
sysctl net.ipv4.tcp_rmem
sysctl net.ipv4.tcp_recovery


tc qdisc add dev vethin2 root tbf rate 10gbit burst 12mbit limit 1000000
tc qdisc add dev vethin2 root tbf rate 11gbit burst 12mbit limit 1000000
tc qdisc add dev vethin2 root tbf rate 12gbit burst 12mbit limit 1000000
tc qdisc add dev vethin2 root tbf rate 12gbit burst 15mbit limit 1000000
tc qdisc add dev vethin2 root tbf rate 10gbit burst 5mbit limit 1000000
tc qdisc del dev vethin2 root


working paras without opera
tc qdisc add dev vethin2 root tbf rate 10gbit burst 5mbit limit 1000000
echo net.ipv4.tcp_rmem = 4096 131072 68000000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 131072 68000000 | tee -a /etc/sysctl.conf
or
with rack
tc qdisc add dev vethin2 root tbf rate 11gbit burst 12mbit limit 1000000
echo net.ipv4.tcp_wmem = 4096 16384 4194304 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_rmem = 4096 131072 6291456 | tee -a /etc/sysctl.conf

for sack
echo net.ipv4.tcp_rmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf
tc qdisc add dev vethin2 root tbf rate 11gbit burst 12mbit limit 1000000
or
tc qdisc add dev vethin2 root tbf rate 11gbit burst 10mbit limit 1000000
echo net.ipv4.tcp_rmem = 4096 6291456 68000000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 6291456 68000000 | tee -a /etc/sysctl.conf
or
tc qdisc add dev vethin2 root tbf rate 11gbit burst 5mbit limit 1000000
echo net.ipv4.tcp_rmem = 4096 6291456 57800000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 6291456 57800000 | tee -a /etc/sysctl.conf

rack - custom srtt
echo net.ipv4.tcp_rmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf
echo net.ipv4.tcp_wmem = 4096 54400000 68000000 | tee -a /etc/sysctl.conf
tc qdisc add dev vethin2 root tbf rate 11gbit burst 5mbit limit 1000000
sysctl -p

//list rate limits
tc q 

cat /proc/sys/net/ipv4/tcp_available_congestion_control
sysctl net.ipv4.tcp_congestion_control
```

### Partial node setup
python3 partial_node_setup.py
python3 partial_setup_arp.py

echo 8 | sudo tee /proc/sys/kernel/printk

ifconfig down/ifconfig up

ifconfig vethin2 up
ifconfig vethin2 down

### Version explanation
```
opera-v1 = encap is done as soon as packets are received from veth or nic and tx side only send the packets out
opera-v2 = rx side simply queue up packets in the correct per dest queues. Tx side does the encap.
opera-ex = experimental
```

### BCC Tools
```
After building from source do the following inside network namespace
mount -t debugfs none /sys/kernel/debug/ 

cd bcc/tools/
python3 tcprtt.py -P 5000

/* original */
/* u32 srtt = ts->srtt_us >> 3; */

    /* RUK: Get Min RTT */
    /* u32 srtt = ts->rtt_min.s[0].v; */

    /* RUK: RACK RTT_US */
    /* u32 srtt = ts->rack.rtt_us; */

    /* RUK: RACK RTT_US */
    /* u32 srtt = ts->rack.reo_wnd_steps; */

    /* RUK: RACK reordering */
    u32 srtt = ts->reordering;
```
