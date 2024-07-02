
### UDP Only Forward Path

```
./udp_server_fwd 5000
./udp_client_fwd 192.168.2.2 5000
```

### UDP Echo
```
./udp_server 5000
./udp_client 192.168.2.2 5000
```

```
./tcp_server 5000
./tcp_client 192.168.2.2 5000

python3 node_health.py -r // read start log
python3 node_health.py -d // delete logs(start_log)
python3 node_health.py -c // check emu_nic process state
```

### Throughput Test
```
sudo ./iperf_tcp_ns_client.sh -n 1 -s 192.168.2.2
sudo ./iperf_tcp_ns_server.sh -n 1 -s 192.168.2.2
```

### TCPDUMP
```
/bin/bash start_tdump.sh
sudo pkill tcpdump
```

### Log Ping results (10us interval between packets)
```
ping 192.168.2.2 -i 0.00001 -c 90000  | while read pong; do echo "$(date):$pong"; done >> /tmp/direct-slot-100us.txt
ping 192.168.2.2 -i 0.00001 -c 60000  | while read pong; do echo "$(date):$pong"; done >> /tmp/opera-slot-100us.txt

ping 192.168.2.2 -i 0.00001 -c 90000  | while read pong; do echo "$(date):$pong"; done >> /tmp/direct-slot-200us.txt
ping 192.168.2.2 -i 0.00001 -c 60000  | while read pong; do echo "$(date):$pong"; done >> /tmp/opera-slot-200us.txt

ping 192.168.2.2 -i 0.00001 -c 90000  | while read pong; do echo "$(date):$pong"; done >> /tmp/direct-slot-1ms.txt
ping 192.168.2.2 -i 0.00001 -c 60000  | while read pong; do echo "$(date):$pong"; done >> /tmp/opera-slot-1ms.txt
```

### Log Congestion Window
```
<!-- bash ss-output.sh 192.168.2.2   -->
./ss-output.sh 192.168.2.2
```
## inside the namespace 
```
mount -t debugfs none /sys/kernel/debug/
cd /sys/kernel/debug/tracing
echo 1 > events/tcp/tcp_probe/enable
<!-- run workloads -->
cat trace

cat /sys/kernel/debug/tracing/trace_pipe
cat /sys/kernel/debug/tracing/trace
```

(https://unix.stackexchange.com/questions/747990/how-to-clear-the-sys-kernel-debug-tracing-trace-pipe-quickly) 
## This will clear both trace and trace_pipe files (might not need)
```
echo > /sys/kernel/debug/tracing/trace
```

## Disable trace
```
cd /sys/kernel/debug/tracing
echo 0 > events/tcp/tcp_probe/enable
```

```
cat trace_pipe >> /tmp/cwnd_logs/direct-no-ratelimit.txt
cat trace_pipe >> /tmp/cwnd_logs/direct-ratelimit.txt
cat trace_pipe >> /tmp/cwnd_logs/opera-rack-default.txt
cat trace_pipe >> /tmp/cwnd_logs/opera-sack-default.txt
cat trace_pipe >> /tmp/cwnd_logs/opera-sack-increase-reo.txt
```

## trace (https://lwn.net/Articles/290277/)
```
timestamp in <secs>.<usecs>
```

```
zip -r sample_file.zip /opt/rotor-xdp
```
