#!/bin/sh

for i in $(seq 1 15000);
do 
    ss -it '( sport = :4000 )' >> /tmp/tcp_stats
    sleep 0.001
done
