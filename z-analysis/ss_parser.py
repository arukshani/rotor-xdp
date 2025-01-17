import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd
import fileinput

# print("hello")

exp_type = "opera"
path = "{}/exp-3-bbr/".format(exp_type)
read_file_name = path+"{}-ss-clean-node-1.txt".format(exp_type)
write_file_name = path+"{}-ss-node-1.csv".format(exp_type)

with open(write_file_name, 'a') as f:
    f.write("{},{},{},{},{}\n".format('time','snd_cwnd','rtt_us','ssthresh','reordering'))
    for line in fileinput.input(read_file_name, encoding="utf-8"):
        # if ('iperf3' in line) and ('192.168.1.2:43520' in line):
        if ('bbr' in line):
            # print(line)
            # line_parts = line.split()
            # time = line_parts[0]
            # snd_cwnd = line_parts[11].split(':')[1]
            # rtt_all = line_parts[6].split(':')[1]
            # rtt_ms = rtt_all.split('/')[0]
            # rtt_us = float(rtt_ms) * 1000 
            # print("{} {} {} \n".format(time,snd_cwnd,rtt_us))


            line_parts = line.split()
            time = line_parts[0]

            # snd_cwnd=0
            cwnd_label = line_parts[9].split(':')[0]
            if (cwnd_label == "cwnd"):
                snd_cwnd = line_parts[9].split(':')[1]

            ssthresh_label = line_parts[10].split(':')[0]
            if (ssthresh_label == "ssthresh"):
                ssthresh = line_parts[10].split(':')[1]
            else:
                ssthresh = 0

            reordering = 0
            for x in line_parts:
                if "reordering" in x: 
                    reordering = x.split(':')[1]

            rtt_all = line_parts[4].split(':')[1]
            rtt_ms = rtt_all.split('/')[0]
            rtt_us = float(rtt_ms) * 1000 
            # print("{} {} {} \n".format(time,snd_cwnd,rtt_us))
            # break
            f.write("{},{},{},{},{}\n".format(time,snd_cwnd,rtt_us,ssthresh,reordering))
f.close()