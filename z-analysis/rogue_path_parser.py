import argparse
import logging
import pandas as pd
from datetime import datetime
import plotly.graph_objs as go
import plotly
import plotly.express as px
import csv
import json
import numpy as np
import matplotlib.pyplot as plt
import re
import numpy as np
import seaborn as sns

path = "/tmp/paper-latency-hop/"
summary_path = "/tmp/paper-latency-hop/"

def read_file(file_name):
    node_df = pd.read_csv(path+file_name ,sep=',')
    return node_df

def check_packet_order_n1(df,file_name):
    f = open(summary_path+file_name, 'w')
    writer = csv.writer(f)
    for i in range(0,100000):
        index_1 = i
        row1=df.iloc[index_1]
        if (row1["slot"] == 0):
            matching_row = df.loc[(df["slot"] == 2) & (df["seq"] == row1["seq"])]
            if (matching_row.empty):
                print("no matching ICMP echo reply for node 1")
            else:
                # n1_send_topo, n2_recv_topo, ret_hop_count, n1_send_seq, n1_recv_seq
                # print(matching_row['topo_arr'].item())
                writer.writerow([row1['topo_arr'].item(), matching_row['topo_arr'].item(), 
                            matching_row['hop_count'].item(), 
                            row1['seq'].item(), matching_row['seq'].item()])        
    f.close()

def check_packet_order_n2(df,file_name):
    f = open(summary_path+file_name, 'w')
    writer = csv.writer(f)
    for i in range(0,100000):
        index_1 = i
        row1=df.iloc[index_1]
        if (row1["slot"] == 2):
            matching_row = df.loc[(df["slot"] == 0) & (df["seq"] == row1["seq"])]
            if (matching_row.empty):
                print("no matching ICMP echo reply for node 2")
            else:
                # n2_recv_topo, n2_send_topo, fwd_hop_count, n2_recv_seq, n2_send_seq
                # print(matching_row['topo_arr'].item())
                writer.writerow([row1['topo_arr'].item(), matching_row['topo_arr'].item(), 
                            row1['hop_count'].item(), 
                            row1['seq'].item(), matching_row['seq'].item()])        
    f.close()
    

# n1_df_100us = read_file("n1-100us-log.csv")
# print(n1_df_100us.count())
# n1_df_100us = n1_df_100us.head(100000)

# n2_df_100us = read_file("n2-100us-log.csv")
# print(n2_df_100us.count())
# n2_df_100us = n2_df_100us.head(100000)

# check_packet_order_n1(n1_df_100us, "n1-topo-track-100us.csv")
# check_packet_order_n2(n2_df_100us, "n2-topo-track-100us.csv")

######

# n1_df_200us = read_file("n1-200us-log.csv")
# print(n1_df_200us.count())

# n2_df_200us = read_file("n2-200us-log.csv")
# print(n2_df_200us.count())

# check_packet_order_n1(n1_df_200us, "n1-topo-track-200us.csv")
# check_packet_order_n2(n2_df_200us, "n2-topo-track-200us.csv")


#####

n1_df_1ms = read_file("n1-1ms-log.csv")
# print(n1_df_1ms.count())

n2_df_1ms = read_file("n2-1ms-log.csv")
# print(n2_df_1ms.count())

check_packet_order_n1(n1_df_1ms, "n1-topo-track-1ms.csv")
check_packet_order_n2(n2_df_1ms, "n2-topo-track-1ms.csv")

