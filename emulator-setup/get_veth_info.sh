#!/bin/sh

# echo $1 $2
# sudo ip netns exec blue arp -i veth0 -s $1 $2
# sudo ip netns exec red arp -i veth2 -s $1 $3


num_namespaces=1 #1 means 2 interfaces
node=$1
# echo $node

# for arg in "$@"
# do
# case $arg in
#     -n|--number-of-ns)
#         shift
#         num_namespaces=$1
#         shift
#         ;;
# esac
# done

# myArray=("blue" "red" "ns12" "ns13" "ns15" "ns16" "ns17" "ns18" "ns19" "ns20" "ns21" "ns22" "ns23" "ns24")
# myArray=("cr1" "cr2" "cr3" "cr4" "cr5" "cr6" "cr7" "cr8")
# myArray=("ns1" "ns2" "ns3" "ns4" "ns5" "ns6" "ns7" "ns8")
myArray=("ns1" "ns2")

for i in $(seq 0 $num_namespaces); do
    ip=$(sudo ip netns exec ${myArray[$i]} ifconfig | awk '/inet / {print $2}')
    ether=$(sudo ip netns exec ${myArray[$i]} ifconfig | awk '/ether / {print $2}')
    iface=$(sudo ip netns exec ${myArray[$i]} netstat -i | grep '^[a-z]' | awk '{print $1}' | grep -v 'lo')
    # echo $i, $node, ${myArray[$i]}, $iface, $ip, $ether
    echo $i, $node, ${myArray[$i]}, $iface, $ip, $ether >> /tmp/NS$node.csv
done