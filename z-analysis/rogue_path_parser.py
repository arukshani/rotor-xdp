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

path = "/tmp/with_icmp_seq/"
summary_path = "/tmp/with_icmp_seq/"

def read_file(file_name):
    node_df = pd.read_csv(path+file_name ,sep=',')
    return node_df

def check_packet_order_n1(df):
    f = open(summary_path+"n1-topo-track-100us.csv", 'w')
    writer = csv.writer(f)
    for i in range(0,100000,2):
        index_1 = i
        row1=df.iloc[index_1]
        index_2 = index_1+1
        row2=df.iloc[index_2]
        if (row1['slot'] == 0 and row2['slot'] == 2):
        # n1_send_topo, n2_recv_top, ret_hop_count, n1_send_seq, n1_recv_seq
            writer.writerow([row1['topo_arr'], row2['topo_arr'], 
                            row2['hop_count'], 
                            row1['seq'], row2['seq']])
        else:
            print("out of order icmp packets noticed for node 1")
    f.close()

def check_packet_order_n2(df):
    f = open(summary_path+"n2-topo-track-100us.csv", 'w')
    writer = csv.writer(f)
    for i in range(0,100000,2):
        index_1 = i
        row1=df.iloc[index_1]
        index_2 = index_1+1
        row2=df.iloc[index_2]
        if (row1['slot'] == 2 and row2['slot'] == 0):
            # n2_recv_topo, n2_send_top, fwd_hop_count, n2_recv_seq, n2_send_seq
            writer.writerow([row1['topo_arr'], row2['topo_arr'], 
                            row1['hop_count'], 
                            row1['seq'], row2['seq']])
        else:
            print("out of order icmp packets noticed for node 1")
    f.close()
    

n1_df_100us = read_file("n1-100us-log.csv")
n1_df_100us = n1_df_100us.head(100000)

n2_df_100us = read_file("n2-100us-log.csv")
n2_df_100us = n2_df_100us.head(100000)

check_packet_order_n1(n1_df_100us)
check_packet_order_n2(n2_df_100us)

