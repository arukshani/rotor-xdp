#!/bin/sh

for arg in "$@"
do
case $arg in
    -f|--file_name)
        shift
        file_name=$1
        shift
        ;;
esac
done

path="/tmp/TDUMPS/2024-04-12_17-25-41/"
path_csv="/tmp/TDUMPS/2024-04-12_17-25-41/CSV/"

tshark -2 -r $path/$file_name.pcap -R "udp.dstport == 12101" -T fields -e frame.time -e ip.src -e ip.dst > $path_csv/$file_name.csv