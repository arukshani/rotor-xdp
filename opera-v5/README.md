```
sudo ip netns exec blue bash
sudo ethtool -L ens4 combined 16

sudo ethtool -G ens4 rx 2048
sudo ethtool -G ens4 tx 2048
sudo ethtool -g ens4
```


### This is version 5: Map per dest queues to NIC queues
```
cd /home/dathapathu/emulator/github_code/opera-xdp
cd /home/dathapathu/emulator/github_code/opera-xdp/opera-v5
cd /home/dathapathu/emulator/github_code/opera-xdp/opera-test-tools

sudo taskset --cpu-list 15 ./emulator_v5 192.168.1.1 configs/node-1-link.csv /dev/ptp0 120 1 1 
sudo taskset --cpu-list 15 ./emulator_v5 192.168.1.2 configs/node-2-link.csv /dev/ptp0 120 1 1 

sudo taskset --cpu-list 31 ./emulator_v5 10.1.0.1 configs/node-1-link.csv /dev/ptp0 120 8 16
sudo taskset --cpu-list 31 ./emulator_v5 10.1.0.2 configs/node-2-link.csv /dev/ptp0 120 8 16 
sudo taskset --cpu-list 31 ./emulator_v5 10.1.0.3 configs/node-3-link.csv /dev/ptp0 120 8 16
sudo taskset --cpu-list 31 ./emulator_v5 10.1.0.4 configs/node-4-link.csv /dev/ptp0 120 8 16

sudo taskset --cpu-list 31 ./exp_sep_threads 10.1.0.1 configs/node-1-link.csv /dev/ptp0 120 8 16
sudo taskset --cpu-list 31 ./exp_sep_threads 10.1.0.2 configs/node-2-link.csv /dev/ptp0 120 8 16 
sudo taskset --cpu-list 31 ./exp_sep_threads 10.1.0.3 configs/node-3-link.csv /dev/ptp0 120 8 16
sudo taskset --cpu-list 31 ./exp_sep_threads 10.1.0.4 configs/node-3-link.csv /dev/ptp0 120 8 16

sudo ./emulator_v5 10.1.0.1 configs/node-1-link.csv /dev/ptp0 120 8 16
sudo ./emulator_v5 10.1.0.2 configs/node-2-link.csv /dev/ptp0 120 8 16

```

