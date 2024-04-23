
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
```

### Throughput Test
```
sudo ./iperf_tcp_ns_client.sh -n 1 -s 192.168.2.2
sudo ./iperf_tcp_ns_server.sh -n 1 -s 192.168.2.2
```


