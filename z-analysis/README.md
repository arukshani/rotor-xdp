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
cat direct-iperf-ss-node-1.txt | sed -e '/48406/ { N; d; }' | less >> direct-iperf-ss-clean-node-1.txt
```