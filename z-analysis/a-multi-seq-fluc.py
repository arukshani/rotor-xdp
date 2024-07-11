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

def get_elapsed_time(topo_time, df_starting_time):
   return topo_time - df_starting_time

path = "throughput/multi-plots/"
plotname = 'throughput/multi-plots/seq-t249000-250000.png'

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

    return n1_df

direct_df = read_file(path+"2-direct-seq-node-1.csv", 56166)
dir_mask = (direct_df['elaps_time_us'] > 249000) & (direct_df['elaps_time_us'] <= 250000)
direct_df = direct_df.loc[dir_mask]

opera_df = read_file(path+"3-opera-seq-node-1.csv", 42300)
opera_mask = (opera_df['elaps_time_us'] > 249000) & (opera_df['elaps_time_us'] <= 250000)
opera_df = opera_df.loc[opera_mask]

plt.plot(opera_df['elaps_time_us'], opera_df['relative_seq'], label = "opera", marker='|')
plt.plot(direct_df['elaps_time_us'], direct_df['relative_seq'], label = "direct", marker='|')


plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('Time (us)', fontsize=11)
plt.ylabel('Relative Sequence Number', fontsize=11)
plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

# fig = go.Figure()
# fig.add_traces(go.Scatter(x=direct_df['elaps_time_us'], y = direct_df['relative_seq'], name="direct", mode='markers'))
# fig.add_traces(go.Scatter(x=sack_df['elaps_time_us'], y = sack_df['relative_seq'], name="sac-opera", mode='markers'))
# fig.write_html("multi-plots/plots/rel-seq-all.html")

