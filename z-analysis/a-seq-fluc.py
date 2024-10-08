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

def read_file(n1_file_name, filter_port):
    n1_df = pd.read_csv(n1_file_name ,sep=',')
    # n1_df = n1_df.loc[(n1_df['slot'] == 0)]
    n1_df = n1_df.loc[(n1_df['src_port'] == filter_port)]

    pos = n1_df.columns.get_loc('time_ns')
    n1_df['elaps_time_ns'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
    n1_df['elaps_time_us'] =  n1_df['elaps_time_ns'] /1000
    n1_df.replace(np.nan, 0, inplace=True)

    seq_pos = n1_df.columns.get_loc('seq')
    n1_df['relative_seq'] =  n1_df.iloc[1:, seq_pos] - n1_df.iat[0, seq_pos]
    n1_df.replace(np.nan, 0, inplace=True)

    # n1_df['seq_change'] = n1_df['seq'].diff()
    n1_df.relative_seq = n1_df.relative_seq+np.where(n1_df.relative_seq<0, 4294967295, 0)
    # n1_df['relative_seq'] = n1_df.seq_change.cumsum()
    # n1_df['seq_change'] = n1_df['seq_change'].add(4294967295).mod(4294967295).sub(4294967295)
    # n1_df['relative_seq'] = n1_df.seq_change.cumsum()

    # n1_df['relative_seq'] =  4294967295 + n1_df['relative_seq']

    # mask = (n1_df['elaps_time_us'] > 0) & (n1_df['elaps_time_us'] <= 600000)
    # n1_df = n1_df.loc[mask]
    # n1_df = n1_df.head(1000) #0-1200
    # n1_df = n1_df[5001:10000] 
    return n1_df

exp_type = "opera"
path = "{}/exp-2/".format(exp_type)
plot_path = path+"/plots/"

from_time = 0
to_time = 0
# to_time = 1000000

if (to_time == 0):
    seq_plot_name = "{}-seq-node-1.png".format(exp_type)
else:
    seq_plot_name = "{}-seq-t{}-{}-node-1.png".format(exp_type, from_time, to_time)


exp_df = read_file(path+"opera-lbuff-node-1.csv", 4000)
plt.plot(exp_df['elaps_time_us'], exp_df['relative_seq'], label = "seq")



# df_new = direct_df[direct_df['relative_seq'] == 0]
# print(df_new)
# topo_direct = pd.read_csv(path+"1-direct-topochange-node-1.csv" ,sep=',')

# df_starting_time = direct_df['time_ns'].iloc[0]
# topo_starting_time = topo_direct[topo_direct.time_ns < df_starting_time].iloc[-1]['time_ns']
# topo_filtered_data = topo_direct[topo_direct.time_ns > topo_starting_time]

## Elapsed time since seq data starting time
# topo_filtered_data['elaps_time_ns'] = topo_filtered_data.apply(lambda row: get_elapsed_time(row['time_ns'], df_starting_time), axis=1)
# topo_filtered_data['elaps_time_us'] =  topo_filtered_data['elaps_time_ns'] /1000
# print(topo_filtered_data.head(10))

# dir_mask = (direct_df['elaps_time_us'] > 249000) & (direct_df['elaps_time_us'] <= 250000)
# direct_df = direct_df.loc[dir_mask]

# topo_mask = (topo_filtered_data['elaps_time_us'] > 249000) & (topo_filtered_data['elaps_time_us'] <= 250000)
# topo_filtered_data = topo_filtered_data.loc[topo_mask]

# plt.plot(direct_df['elaps_time_us'], direct_df['relative_seq'], label = "seq")
# plt.plot(sack_df['elaps_time_us'], sack_df['relative_seq'], label = "sack-opera", marker='|')

# periods_topo = topo_filtered_data.elaps_time_us
# periods_label = topo_filtered_data.curr_topo
# periods_indices = topo_filtered_data.index.values
# for item in periods_topo:
#     plt.axvline(item, ymin=0, ymax=1,color='red')

# for i in range(len(periods_indices)):
#     index = periods_indices[i]
#     plt.text(y=direct_df['relative_seq'].max(),x=(periods_topo[index]),
#         s=str(periods_label[index]), color='black', fontsize=7.5)

# periods_dir = direct_df[direct_df.topo_arr.diff()!=0].elaps_time_us
# labels = sack_df[sack_df.topo_arr.diff()!=0].topo_arr
# indices = sack_df[sack_df.topo_arr.diff()!=0].index.values
# for item in periods_dir:
#     plt.axvline(item, ymin=0, ymax=1,color='black')

# for i in range(len(indices)):
#     index = indices[i]
#     # next_index = indices[i+1]
#     plt.text(y=sack_df['relative_seq'].max(),x=(periods[index]),
#         s=str(labels[index]), color='black', fontsize=10)

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('Time (us)', fontsize=11)
plt.ylabel('Relative Sequence Number', fontsize=11)
plt.savefig(plot_path+seq_plot_name)


# plt.savefig('P-ALL-RTTs.pdf')

# fig = go.Figure()
# fig.add_traces(go.Scatter(x=direct_df['elaps_time_us'], y = direct_df['relative_seq'], name="direct", mode='markers'))
# fig.add_traces(go.Scatter(x=sack_df['elaps_time_us'], y = sack_df['relative_seq'], name="sac-opera", mode='markers'))
# fig.write_html("multi-plots/plots/rel-seq-all.html")

