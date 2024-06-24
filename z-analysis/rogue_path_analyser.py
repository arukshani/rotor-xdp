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

summary_path = "/tmp/paper-latency-hop/"

def read_file(n1_file_name, n2_file_name):
    n1_df = pd.read_csv(summary_path+n1_file_name ,sep=',', names=["n1_send_topo", "n1_recv_topo", 
                                                        "ret_hop_count", "n1_send_seq", "n1_recv_seq"])
    n2_df = pd.read_csv(summary_path+n2_file_name ,sep=',', names=["n2_recv_topo", "n2_send_topo", 
                                                        "fwd_hop_count", "n2_recv_seq", "n2_send_seq"])
    
    combined_df = pd.concat([n1_df, n2_df], axis=1)
    # print(combined_df.head(5))
    return combined_df

def get_rogue_path_count(combined_df):
    rogue_path_packet_count = 0
    for i in range(0,50000):
        row = combined_df.iloc[i]
        if (row['n1_send_seq'] == row['n1_recv_seq'] == row['n2_recv_seq'] == row['n2_send_seq']):
            if (row['n1_send_topo'] != row['n2_recv_topo']):
                rogue_path_packet_count = rogue_path_packet_count + 1
            if (row['n2_send_topo'] != row['n1_recv_topo']):
                rogue_path_packet_count = rogue_path_packet_count + 1
        else: 
            print("Seq numbers don't match")
    return rogue_path_packet_count

combined_1ms_df = read_file("n1-topo-track-1ms.csv", "n2-topo-track-1ms.csv")
# print(combined_1ms_df.count())
# print(combined_1ms_df.head(5))
rogue_count_1ms=get_rogue_path_count(combined_1ms_df)
rogue_perc_1ms = (rogue_count_1ms/100000) * 100
print(rogue_perc_1ms) #2.648

combined_100us_df = read_file("n1-topo-track-100us.csv", "n2-topo-track-100us.csv")
# print(combined_1ms_df.count())
# print(combined_1ms_df.head(5))
rogue_count_100us=get_rogue_path_count(combined_100us_df)
rogue_perc_100us = (rogue_count_100us/100000) * 100
print(rogue_perc_100us) #29.469

combined_200us_df = read_file("n1-topo-track-200us.csv", "n2-topo-track-200us.csv")
# print(combined_1ms_df.count())
# print(combined_1ms_df.head(5))
rogue_count_200us=get_rogue_path_count(combined_200us_df)
rogue_perc_200us = (rogue_count_200us/100000) * 100
print(rogue_perc_200us) #12.726



