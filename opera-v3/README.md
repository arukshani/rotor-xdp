### Emulator Opera - 2 Node

```
sudo ./emu_nic 192.168.1.1 indirect-3node-config/node-1.csv /dev/ptp3 120 1 1 
sudo ./emu_nic 192.168.2.1 indirect-3node-config/node-2.csv /dev/ptp3 120 1 1 
sudo ./emu_nic 192.168.3.1 indirect-3node-config/node-3.csv /dev/ptp3 120 1 1 

sudo ./emu_nic 192.168.1.1 direct-2node-config/node-1.csv /dev/ptp3 120 1 1 
sudo ./emu_nic 192.168.2.1 direct-2node-config/node-2.csv /dev/ptp3 120 1 1 
sudo ./emu_nic 192.168.3.1 direct-2node-config/node-3.csv /dev/ptp3 120 1 1 

sudo taskset --cpu-list 1 ./emu_nic 192.168.1.1 config/node-1.csv /dev/ptp3 60 1 1 
sudo taskset --cpu-list 1 ./emu_nic 192.168.2.1 config/node-2.csv /dev/ptp3 60 1 1
```

```
ss --tcp  -i
ss -at '( dport = :22 or sport = :22 )'


iperf3 -c 192.168.2.2 -p 5000 --cport 4000  -t 60
iperf3 -s -p 5000
ss -it '( sport = :4000 )'
netstat -s | egrep -i "(segments send out|retrans|TCPLossProbe)"
```

### ssh into namespace
```
sudo ip netns exec ns1 bash
sudo ip netns exec ns2 bash
```