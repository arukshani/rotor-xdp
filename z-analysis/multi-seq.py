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

path = "multi-plots/"
plotname = 'multi-plots/plots/relative-seq-time1001-2000.png'

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

    mask = (n1_df['elaps_time_us'] > 5750) & (n1_df['elaps_time_us'] <= 6250)
    n1_df = n1_df.loc[mask]
    # n1_df = n1_df.head(1000) #0-1200
    # n1_df = n1_df[5001:10000] 
    return n1_df

direct_df = read_file(path+"4-direct-node-1.csv", 55006)
sack_df = read_file(path+"2-seq-sack-node-1.csv", 43008)


plt.plot(direct_df['elaps_time_us'], direct_df['relative_seq'], label = "direct", marker='|')
plt.plot(sack_df['elaps_time_us'], sack_df['relative_seq'], label = "sack-opera", marker='|')

periods = sack_df[sack_df.topo_arr.diff()!=0].elaps_time_us
labels = sack_df[sack_df.topo_arr.diff()!=0].topo_arr
indices = sack_df[sack_df.topo_arr.diff()!=0].index.values
for item in periods:
    plt.axvline(item, ymin=0, ymax=1,color='red')

for i in range(len(indices)):
    index = indices[i]
    # next_index = indices[i+1]
    plt.text(y=sack_df['relative_seq'].max(),x=(periods[index]),
        s=str(labels[index]), color='black', fontsize=10)

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time (us)', fontsize=11)
plt.ylabel('relative sequence number', fontsize=11)
plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

# fig = go.Figure()
# fig.add_traces(go.Scatter(x=direct_df['elaps_time_us'], y = direct_df['relative_seq'], name="direct", mode='markers'))
# fig.add_traces(go.Scatter(x=sack_df['elaps_time_us'], y = sack_df['relative_seq'], name="sac-opera", mode='markers'))
# fig.write_html("multi-plots/plots/rel-seq-all.html")

