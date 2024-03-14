### Master Node (Last node of the cluster)

```
scp ~/.ssh/rukshani_cloudlab.pem rukshani@ip:~/.ssh/
chown -R rukshani /opt
cd /opt
got clone https://github.com/arukshani/rotor-xdp.git
cd /opt/rotor-xdp/emulator-setup
./setup_master.sh
python3 setup_cloudlab.py
python3 setup_arp.py
```



### if you want to delete all arp records
```
for e in $(arp -a | sed -n 's/.*(\([^()]*\)).*/\1/p'); do arp -d $e; done
```
