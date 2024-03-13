
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

cd /home/dathapathu/emulator/github_code/opera-xdp/v6-multi-pool
cd /home/dathapathu/emulator/github_code/opera-xdp/opera-test-tools

sudo taskset --cpu-list 32 ./sw_corundum_1 10.20.1.1 120 3 3
sudo taskset --cpu-list 32 ./sw_corundum_1 10.20.2.1 120 3 3

sudo taskset --cpu-list 18 ./sw_corundum_main 10.20.1.1 120 7 7 config/node2.csv
sudo taskset --cpu-list 18 ./sw_corundum_main 10.20.2.1 120 7 7 config/node1.csv

sudo ./uq_tcp_ns_client.sh -n 0 -i ens2np0
sudo ./uq_tcp_ns_server.sh -n 0 -i ens2np0

sudo ./uq_mp_server.sh -n 5
sudo ./uq_mp_client.sh -n 5

sudo taskset --cpu-list 22 ./sw_corundum_main 10.20.1.1 120 1 1 config/node2.csv
sudo taskset --cpu-list 22 ./sw_corundum_main 10.20.2.1 120 1 1 config/node1.csv

sudo ./uq_mp_server.sh -n 0
sudo ./uq_mp_client.sh -n 0

sudo ethtool -L ens2np0 rx 1
sudo ethtool -L ens2np0 tx 1

sudo set_irq_affinity.sh ens2np0

sudo ip netns exec ns1 bash
```

```
IRQ Affinity
https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/html/performance_tuning_guide/s-cpu-irq

 echo 0000,00000000,00000001 | sudo tee /proc/irq/196/smp_affinity
 echo 0000,00000000,00000004 | sudo tee /proc/irq/197/smp_affinity
 echo 0000,00000000,00000008 | sudo tee /proc/irq/198/smp_affinity
 echo 0000,00000000,00000010 | sudo tee /proc/irq/199/smp_affinity
 echo 0000,00000000,00000020 | sudo tee /proc/irq/200/smp_affinity
 echo 0000,00000000,00000040 | sudo tee /proc/irq/201/smp_affinity
 echo 0000,00000000,00000040 | sudo tee /proc/irq/202/smp_affinity

cat /proc/irq/196/smp_affinity (0000,00000000,00000001)


```

```
sudo ip netns exec ns1 ethtool -K vethin2 tx on
sudo ip netns exec ns2 ethtool -K vethin3 tx on
sudo ip netns exec ns3 ethtool -K vethin4 tx on
sudo ip netns exec ns4 ethtool -K vethin5 tx on
sudo ip netns exec ns5 ethtool -K vethin6 tx on
sudo ip netns exec ns6 ethtool -K vethin7 tx on
sudo ip netns exec ns7 ethtool -K vethin8 tx on
sudo ip netns exec ns8 ethtool -K vethin9 tx on
```

```
cmds = [
    'echo net.core.wmem_max = 16777216 | sudo tee -a /etc/sysctl.conf',
    'echo net.core.rmem_max = 16777216 | sudo tee -a /etc/sysctl.conf',
    'echo net.core.wmem_default = 16777216 | sudo tee -a /etc/sysctl.conf', 
    'echo net.core.rmem_default = 16777216 | sudo tee -a /etc/sysctl.conf',
    'echo net.ipv4.tcp_wmem = 10240 16777216 16777216 | sudo tee -a /etc/sysctl.conf',
    'echo net.ipv4.tcp_rmem = 10240 16777216 16777216 | sudo tee -a /etc/sysctl.conf',
    'sudo sysctl -p'
    ]

    echo net.core.somaxconn = 2048 | sudo tee -a /etc/sysctl.conf
    echo net.ipv4.tcp_max_syn_backlog = 2048 | sudo tee -a /etc/sysctl.conf

    net.core.somaxconn=2048
    net.ipv4.tcp_max_syn_backlog=2048
```

```
Docker
sudo apt update
sudo apt install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu focal stable"
apt-cache policy docker-ce

sudo apt install docker-ce
sudo systemctl status docker
sudo usermod -aG docker ${USER}
groups

docker run hello-world
docker search ubuntu
docker pull ubuntu

docker images
docker run -it ubuntu
docker ps
docker ps -a
docker ps -l
docker stop quizzical_mcnulty
docker rm youthful_curie

docker inspect <container ID>
docker inspect <container id> | grep "IPAddress"
docker inspect -f '{{.Name}} - {{.NetworkSettings.IPAddress }}' $(docker ps -aq)


docker network create --subnet=172.18.0.0/16 mynet123
docker run --net mynet123 --ip 172.18.0.22 -it ubuntu bash

apt-get update
apt-get install -y net-tools
apt install iproute2 -y
apt-get install -y iputils-ping
```

```
docker start 747cd5b917e9
docker stop 747cd5b917e9
docker exec -it <mycontainer> bash


docker network rm operanetwork
```

```
docker network create --subnet=172.20.0.0/16 operanetwork
docker run --net operanetwork --ip 172.20.1.2 -it ubuntu bash
docker run --net operanetwork --ip 172.20.2.2 -it ubuntu bash

apt-get update
apt-get install -y net-tools
apt install iproute2 -y
apt-get install -y iputils-ping
apt-get install iperf3
```
