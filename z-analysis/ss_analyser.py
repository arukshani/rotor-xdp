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

def get_tot(fwd, ret):
    return (fwd+ret)

path = "iperf-data/opera/exp-1/"
plot_path = "iperf-data/opera/exp-1/"
trace_file = "opera-iperf-ss-node-1.csv"
plotname = "opera-iperf-cwnd-t12s-14ss-node-1.png"

# path = "iperf-data/direct/exp-3/"
# plot_path = "iperf-data/direct/exp-3/"
# trace_file = "direct-iperf-ss-node-1.csv"
# plotname = "direct-iperf-cwnd-t0-500ms-node-1.png"

trace_df = pd.read_csv(path+trace_file ,sep=',')
pos = trace_df.columns.get_loc('time')
trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000
# print(trace_df.head(5))

mask = (trace_df['elaps_time_us'] > 12000000) & (trace_df['elaps_time_us'] <= 14000000)
trace_df = trace_df.loc[mask]

plt.plot(trace_df['elaps_time_us'], trace_df['snd_cwnd'], label = "cwnd")
plt.plot(trace_df['elaps_time_us'], trace_df['ssthresh'], label = "ssthresh", linestyle='dashed')
# plt.plot(trace_df['elaps_time_us'], trace_df['rtt_us'], label = "rtt")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time (us)', fontsize=11)
# plt.ylabel('rtt (us)', fontsize=11)
plt.ylabel('cwnd (packets)', fontsize=11)
plt.savefig(plot_path+plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 