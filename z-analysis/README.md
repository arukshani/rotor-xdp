### Telemetry

```
slot 0 - Receive from VETH and going to NIC
slot 1 - Receive from NIC but going back to NIC
slot 2 - Receive from NIC and going to VETH
```

```
scp -o StrictHostKeyChecking=no node-1:/tmp/udp_client_rtt.csv .

## remove control iperf flow
cat 4-opera-iperf-ss-node-1.txt | sed -e '/51720/ { N; d; }' | less >> test.txt
cat direct/exp-2/direct-ss-node-1.txt | sed -e '/41552/ { N; d; }' | less >> direct/exp-2/direct-ss-clean-node-1.txt
cat opera/exp-2/opera-ss-node-1.txt | sed -e '/41554/ { N; d; }' | less >> opera/exp-2/opera-ss-clean-node-1.txt

cat direct/exp-1-bbr/direct-ss-node-1.txt | sed -e '/37414/ { N; d; }' | less >> direct/exp-1-bbr/direct-ss-clean-node-1.txt
cat opera/exp-3-bbr/opera-ss-node-1.txt | sed -e '/37428/ { N; d; }' | less >> opera/exp-3-bbr/opera-ss-clean-node-1.txt
```


python3 ss_parser.py

## cwnd and rtt plot
python3 ss_analyser.py 

q_num,buff_size,time_part_sec,time_part_nsec,time_ns,src_port,seq
curr_topo,time_part_sec,time_part_nsec,time_ns

## local and veth buffer plots
python3 buff_plot.py

