```
sudo ip netns exec blue bash
sudo ethtool -L ens4 combined 1
```


### This is version 4
```
sudo taskset --cpu-list 15 ./opera_emulator_v4 192.168.1.1 configs/node-1-link.csv /dev/ptp0 120 1 1 
sudo taskset --cpu-list 15 ./opera_emulator_v4 192.168.1.2 configs/node-2-link.csv /dev/ptp0 120 1 1 

sudo taskset --cpu-list 11 ./opera_emulator_v4 10.1.0.1 configs/node-1-link.csv /dev/ptp0 120 1 1 
sudo taskset --cpu-list 11 ./opera_emulator_v4 10.1.0.2 configs/node-2-link.csv /dev/ptp0 120 1 1 
```

