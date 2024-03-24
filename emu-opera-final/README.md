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