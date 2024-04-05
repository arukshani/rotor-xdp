#!/bin/sh

sudo ip netns exec ns1 bash
for e in $(arp -a | sed -n 's/.*(\([^()]*\)).*/\1/p'); do arp -d $e; done
exit
sudo ip netns exec ns2 bash
for e in $(arp -a | sed -n 's/.*(\([^()]*\)).*/\1/p'); do arp -d $e; done
exit