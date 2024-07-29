#!/bin/sh

cd iperf-cubic-7/direct-4000/exp-5/
cat direct-ss-node-1.txt | sed -e '/52044/ { N; d; }' | less >> direct-ss-clean-node-1.txt

# cd iperf-cubic-7/opera-4000/exp-5/
# cat opera-ss-node-1.txt | sed -e '/52022/ { N; d; }' | less >> opera-ss-clean-node-1.txt