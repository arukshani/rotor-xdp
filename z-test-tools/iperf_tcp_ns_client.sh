#!/bin/bash

# Actual server is running on the other side on root

num_namespaces=0
bandwidth="50000M"

for arg in "$@"
do
case $arg in
    -n|--number-of-ns)
        shift
        num_namespaces=$1
        shift
        ;;
    -i|--interface)
        shift
        interface=$1
        shift
        ;;
    -s|--ip)
        shift
        ip=$1
        shift
        ;;
esac
done

server=$ip
# nic_local_numa_node=$(cat /sys/class/net/$interface/device/numa_node)

# myArray=("blue" "red" "ns12" "ns13" "ns15" "ns16" "ns17" "ns18" "ns19" "ns20" "ns21" "ns22" "ns23" "ns24")
# myArray=("cr1" "cr2" "cr3" "cr4" "cr5" "cr6" "cr7" "cr8")
myArray=("ns1" "ns2")

cpu_core_id=$(echo "12" | bc)
output=$(
for i in $(seq 0 $num_namespaces); do
    cpu_core_id=$(echo "$cpu_core_id+2" | bc)
    port=$(echo "5100+$i" | bc);
    # echo $cpu_core_id
    # numactl -N $nic_local_numa_node ip netns exec ${myArray[$i]} iperf3 -c $server -p $port -t 30 -f g &
     sudo taskset --cpu-list $cpu_core_id ip netns exec ${myArray[$i]} iperf3 -c $server -p $port -t 30 -f g &
done
)

inter_out=$(echo $output| grep -o -P '(?<=sender).*?(?=receiver)')
sender_total_tput=$(echo $inter_out | grep -Po '[0-9.]*(?= Gbits/sec)' | awk '{sum+=$1} END {print sum}')
echo $sender_total_tput