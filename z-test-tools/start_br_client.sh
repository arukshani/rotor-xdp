#!/bin/sh

for i in {0..9}
do
    # echo $-n "test data";
    echo -n "test data" | nc -u -b 10.10.255.255 12101 &
    pid=$!
    sleep 0.01;

    # echo ===
    # echo PID is $pid, before kill:
    # ps -ef | grep -E "PPID|$pid" | sed 's/^/   /'
    # echo ===

    ( kill -TERM $pid ) 2>&1
done