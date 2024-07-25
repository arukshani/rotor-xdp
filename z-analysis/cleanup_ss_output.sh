#!/bin/sh

# cd iperf-cubic-7/direct/exp-1/
# cat direct-ss-node-1.txt | sed -e '/55824/ { N; d; }' | less >> direct-ss-clean-node-1.txt

cd iperf-cubic-7/opera/exp-4/
cat opera-ss-node-1.txt | sed -e '/34746/ { N; d; }' | less >> opera-ss-clean-node-1.txt