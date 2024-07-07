import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd
import fileinput

# print("hello")

path = "sack-small-file/"
read_file_name = path+"2-cwnd-sack-node-1.txt"
write_file_name = path+"2-cwnd-sack-node-1.csv"

with open(write_file_name, 'a') as f:
    f.write("{},{}\n".format('time','snd_cwnd'))
    for line in fileinput.input(read_file_name, encoding="utf-8"):
        # if ('iperf3' in line) and ('192.168.1.2:43520' in line):
        if ('ts' in line):
            # print(line)
            line_parts = line.split()
            time = line_parts[0]
            snd_cwnd = line_parts[11].split(':')[1]
            # time = line_parts[15][:-1]
            # src = line_parts[18].split('=')[1]
            # dest = line_parts[19].split('=')[1]
            # data_len = line_parts[21].split('=')[1]
            # snd_nxt = line_parts[22].split('=')[1]
            # snd_una = line_parts[23].split('=')[1]
            # snd_cwnd = line_parts[24].split('=')[1]
            # ssthresh = line_parts[25].split('=')[1]
            # snd_wnd = line_parts[26].split('=')[1]
            # srtt = line_parts[27].split('=')[1]
            # rcv_wnd = line_parts[28].split('=')[1]
            # # print ("{},{},{},{},{},{},{},{},{},{},{}\n".format(time,src,dest,data_len,snd_nxt,snd_una,snd_cwnd,ssthresh,
            # #                     snd_wnd,srtt,rcv_wnd))
            f.write("{},{}\n".format(time,snd_cwnd))
f.close()