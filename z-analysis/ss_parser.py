import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd
import fileinput

# print("hello")

path = "iperf-data/exp-1/"
read_file_name = path+"1-direct-iperf-ss-clean-node-1.txt"
write_file_name = path+"1-direct-iperf-ss-node-1.csv"

with open(write_file_name, 'a') as f:
    f.write("{},{},{}\n".format('time','snd_cwnd','rtt_us'))
    for line in fileinput.input(read_file_name, encoding="utf-8"):
        # if ('iperf3' in line) and ('192.168.1.2:43520' in line):
        if ('cubic' in line):
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
            snd_cwnd = line_parts[9].split(':')[1]
            rtt_all = line_parts[4].split(':')[1]
            rtt_ms = rtt_all.split('/')[0]
            rtt_us = float(rtt_ms) * 1000 
            # print("{} {} {} \n".format(time,snd_cwnd,rtt_us))
            # break
            f.write("{},{},{}\n".format(time,snd_cwnd,rtt_us))
f.close()