
### This is version 5 refactoring: corundum test
```
sudo taskset --cpu-list 32 ./sw_corundum 10.20.1.1 120 8 16
sudo taskset --cpu-list 32 ./sw_corundum 10.20.2.1 120 8 16 

sudo ./iperf_tcp_ns_client.sh -n 0 -i ens2np0 -s 10.20.2.2
sudo ./iperf_tcp_ns_server.sh -n 0 -i ens2np0 -s 10.20.2.2

iperf3 -c 10.20.2.2 -p 5000
iperf3 -s 10.20.2.2 -p 5000

sudo ethtool -L ens2np0 rx 16
sudo ethtool -L ens2np0 tx 16

sudo ip link set dev ens2np0 up
sudo ip link set dev ens2np0 down

sudo set_irq_affinity.sh ens2np0

```

```
sudo ip addr del 10.20.2.1/24 dev ens2np0
sudo ip addr del 10.20.2.2/24 dev ens2np0

sudo ip addr add 10.20.1.1/16 dev ens2np0
sudo ip addr add 10.20.2.1/16 dev ens2np0

sudo ip link set dev ens2np0 up
sudo ip link set ens2np0 mtu 3490

```

```

cd /home/dathapathu/emulator/github_code/opera-xdp/v5-no-reserve-corundum

cd /home/dathapathu/emulator/github_code/opera-xdp/test-corundum
cd /home/dathapathu/emulator/github_code/opera-xdp/opera-test-tools

sudo taskset --cpu-list 32 ./sw_corundum_1 10.20.1.1 120 3 3
sudo taskset --cpu-list 32 ./sw_corundum_1 10.20.2.1 120 3 3

sudo taskset --cpu-list 22 ./sw_corundum_1 10.20.1.1 120 7 7 config/node2.csv
sudo taskset --cpu-list 22 ./sw_corundum_1 10.20.2.1 120 7 7 config/node1.csv

sudo ./uq_tcp_ns_client.sh -n 0 -i ens2np0
sudo ./uq_tcp_ns_server.sh -n 0 -i ens2np0

sudo ./uq_mp_server.sh -n 0
sudo ./uq_mp_client.sh -n 0

sudo ethtool -L ens2np0 rx 1
sudo ethtool -L ens2np0 tx 1
sudo set_irq_affinity.sh ens2np0
```

```
sudo taskset --cpu-list 14 ./sw_corundum_1 10.20.1.1 120 8 8 config/node2.csv
sudo taskset --cpu-list 14 ./sw_corundum_1 10.20.2.1 120 8 8 config/node1.csv
```

```
sudo ip netns exec ns1 bash
cat /sys/class/net/vethin2/queues/rx-0/rps_cpus
echo 00,00000000,00200000 | sudo tee /sys/class/net/vethin2/queues/rx-0/rps_cpus

echo 00,00000000,20000000 | tee /sys/class/net/vethin2/queues/rx-0/rps_cpus


echo 00,00000000,00000000 | tee /sys/class/net/vethin2/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin3/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin4/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin5/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin6/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin7/queues/rx-0/rps_cpus
echo 00,00000000,00000000 | tee /sys/class/net/vethin8/queues/rx-0/rps_cpus
```

```
cd /opt/opera-xdp/opera-test-tools
cd /opt/opera-xdp/v5-no-reserve-corundum
cd /opt/opera-xdp/opera-setup-leed

sudo taskset --cpu-list 18 ./sw_corundum_1 10.20.1.1 120 1 1 config/node2.csv
sudo taskset --cpu-list 18 ./sw_corundum_1 10.20.2.1 120 1 1 config/node1.csv

sudo ./uq_mp_server.sh -n 0
sudo ./uq_mp_client.sh -n 0

sudo ethtool -L enp175s0np1 rx 1
sudo ethtool -L enp175s0np1 tx 1
sudo ./set_irq_affinity.sh enp175s0np1

sudo ethtool -G enp175s0np1 rx 2048
sudo ethtool -G enp175s0np1 tx 2048
sudo ethtool -g enp175s0np1

echo 2| sudo tee /sys/class/net/enp175s0np1/napi_defer_hard_irqs
echo 200000 | sudo tee /sys/class/net/enp175s0np1/gro_flush_timeout

sudo ethtool -N enp175s0np1 flow-type udp4 dst-port 8888 action 4
```