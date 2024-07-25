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
cat direct-ss-node-1.txt | sed -e '/55824/ { N; d; }' | less >> direct-ss-clean-node-1.txt
cat opera-ss-node-1.txt | sed -e '/45650/ { N; d; }' | less >> opera-ss-clean-node-1.txt
```