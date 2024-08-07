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

path = "throughput/direct-100/seq_data/"
filename = "2-direct100-seq-node-1.csv"
plotname = 'throughput/direct-100/direct100-seq-all.html'

def read_file(n1_file_name):
    n1_df = pd.read_csv(path+n1_file_name ,sep=',')
    return n1_df

n1_df = read_file(filename)
n1_df = n1_df.loc[(n1_df['slot'] == 0)]
n1_df = n1_df.loc[(n1_df['src_port'] == 33696)]

pos = n1_df.columns.get_loc('time_ns')
n1_df['elaps_time_ns'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
n1_df['elaps_time_us'] =  n1_df['elaps_time_ns'] /1000
n1_df.replace(np.nan, 0, inplace=True)
# n1_df = n1_df.head(1000) #0-1200
# n1_df = n1_df[1001:2000] 

# print(n1_df[['seq', 'topo_arr', 'elaps_time_us']])

# periods = n1_df[n1_df.topo_arr.diff()!=0].elaps_time_us
# labels = n1_df[n1_df.topo_arr.diff()!=0].topo_arr
# indices = n1_df[n1_df.topo_arr.diff()!=0].index.values

# plt.plot(n1_df['elaps_time_us'], n1_df['seq'], label = "direct", marker='|')
# for item in periods:
#     plt.axvline(item, ymin=0, ymax=1,color='red')

# for i in range(len(indices)):
#     index = indices[i]
#     # next_index = indices[i+1]
#     plt.text(y=n1_df['seq'].max(),x=(periods[index]),
#         s=str(labels[index]), color='black', fontsize=10)

# ax = sns.lineplot(x="elaps_time_us", y="seq", data=n1_df)
# ax.axvspan(3, 100,facecolor="darkmagenta", edgecolor='black', hatch="o", alpha=.3, label="Easter week")

# plt.legend(fontsize=11)
# plt.xticks(fontsize=11)
# plt.yticks(fontsize=11)
# plt.xlabel('time (us)', fontsize=11)
# plt.ylabel('sequence number', fontsize=11)
# plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

fig = go.Figure()
fig.add_traces(go.Scatter(x=n1_df['elaps_time_us'], y = n1_df['seq'], name="direct", mode='markers'))
fig.write_html(plotname)

