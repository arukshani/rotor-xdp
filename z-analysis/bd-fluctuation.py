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

path = "throughput-2/exp-2/"
plotname = "throughput-2/exp-2/throughput_compare_1000w.png"

BYTES_TO_BITS = 8
BITS_TO_MEGABITS = 1.0 / 1000000.0
MILLISECONDS_TO_SECONDS = 1.0 / 1000.0
US_TO_SECONDS = 1.0 / 1000000.0
BYTES_TO_MEGABYTES = 1.0 / 1000000.0
BITS_TO_GIGABITS = 1.0 / 1000000000.0

# def toMb(x):
#    return (x * cmn.BYTES_TO_BITS) * cmn.BITS_TO_MEGABITS

# def bitrate(x, window_size):
#    return x / (window_size * cmn.US_TO_SECONDS)

def get_throughput(num_bytes, window_size_us):
   return (num_bytes * BYTES_TO_BITS * BITS_TO_GIGABITS) / (window_size_us * US_TO_SECONDS)

def read_file(n1_file_name, filter_port):
    n1_df = pd.read_csv(n1_file_name ,sep=',')
    n1_df = n1_df.loc[(n1_df['slot'] == 0)]
    n1_df = n1_df.loc[(n1_df['src_port'] == filter_port)]

    pos = n1_df.columns.get_loc('time_ns')
    n1_df['elaps_time_ns'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
    n1_df['elaps_time_us'] =  n1_df['elaps_time_ns'] /1000
    n1_df.replace(np.nan, 0, inplace=True)

    seq_pos = n1_df.columns.get_loc('seq')
    n1_df['relative_seq'] =  n1_df.iloc[1:, seq_pos] - n1_df.iat[0, seq_pos]
    n1_df.replace(np.nan, 0, inplace=True)

    # mask = (n1_df['elaps_time_us'] > 36000) & (n1_df['elaps_time_us'] <= 42000)
    # n1_df = n1_df.loc[mask]
    # n1_df = n1_df.head(1000) #0-1200
    # n1_df = n1_df[5001:10000] 

    n1_df = n1_df.iloc[::1000, :]
    n1_df['time_change'] = n1_df['elaps_time_us'].diff()
    n1_df['seq_change'] = n1_df['relative_seq'].diff()
    n1_df.replace(np.nan, 0, inplace=True)
    n1_df['throughput'] = n1_df.apply(lambda row: get_throughput(row['seq_change'], row['time_change']), axis=1)
    n1_df.replace(np.nan, 0, inplace=True)

    return n1_df

direct_df = read_file(path+"2-direct-seq-node-1.csv", 46440)
sack_df = read_file(path+"2-opera-seq-node-1.csv", 46022)

# print(direct_df[['ns_packet_len', 'relative_seq', 'elaps_time_us']].head(10))

plt.plot(direct_df['elaps_time_us'], direct_df['throughput'], label = "direct", marker='|')
plt.plot(sack_df['elaps_time_us'], sack_df['throughput'], label = "opera", marker='|')

# periods = sack_df[sack_df.topo_arr.diff()!=0].elaps_time_us
# labels = sack_df[sack_df.topo_arr.diff()!=0].topo_arr
# indices = sack_df[sack_df.topo_arr.diff()!=0].index.values
# for item in periods:
#     plt.axvline(item, ymin=0, ymax=1,color='red')

# for i in range(len(indices)):
#     index = indices[i]
#     # next_index = indices[i+1]
#     plt.text(y=sack_df['throughput'].max(),x=(periods[index]),
#         s=str(labels[index]), color='black', fontsize=10)

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time (us)', fontsize=11)
plt.ylabel('Throughput (Gbps)', fontsize=11)
plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

# fig = go.Figure()
# fig.add_traces(go.Scatter(x=direct_df['elaps_time_us'], y = direct_df['relative_seq'], name="direct", mode='markers'))
# fig.add_traces(go.Scatter(x=sack_df['elaps_time_us'], y = sack_df['relative_seq'], name="sac-opera", mode='markers'))
# fig.write_html("multi-plots/plots/rel-seq-all.html")

