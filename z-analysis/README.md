### Telemetry

```
slot 0 - Receive from VETH and going to NIC
slot 1 - Receive from NIC but going back to NIC
slot 2 - Receive from NIC and going to VETH
```

```
scp -o StrictHostKeyChecking=no node-1:/tmp/udp_client_rtt.csv .
```