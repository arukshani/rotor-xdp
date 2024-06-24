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

summary_path = "/tmp/with_icmp_seq/"


n1_df = pd.read_csv(summary_path+"n1-topo-track-100us.csv" ,sep=',', names=["n1_send_topo", "n1_recv_topo", 
                                                        "ret_hop_count", "n1_send_seq", "n1_recv_seq"])
n2_df = pd.read_csv(summary_path+"n2-topo-track-100us.csv" ,sep=',', names=["n2_recv_topo", "n2_send_top", 
                                                        "fwd_hop_count", "n2_recv_seq", "n2_send_seq"])

print(n1_df.head(5))
print(n2_df.head(5))

# print(n1_df.count())
# print(n2_df.count())

rogue_path_packet_count = 0

for i in range(0,50000):
    n1_row=n1_df.iloc[i]
    n2_row=n2_df.iloc[i]
    if (n1_row['n1_send_seq'] == n1_row['n1_recv_seq'] == n2_row['n2_recv_seq'] == n2_row['n2_send_seq']):
        if not (n1_row['n1_send_topo'] == n1_row['n1_recv_topo'] == n2_row['n2_recv_topo'] == n2_row['n2_send_top']):
            rogue_path_packet_count = rogue_path_packet_count + 1
    else: 
        print("Seq numbers are not the same")

print(rogue_path_packet_count)


