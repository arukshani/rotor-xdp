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

path = "throughput-2/exp-2/"
plot_path = "throughput-2/exp-2/"

trace_file = "2-direct-ss-node-1.csv"
plotname = "2-direct-rtt-node-1.png"

trace_df = pd.read_csv(path+trace_file ,sep=',')
pos = trace_df.columns.get_loc('time')
trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000
# print(trace_df.head(5))

# plt.plot(trace_df['elaps_time_us'], trace_df['snd_cwnd'], label = "direct-cwnd", marker="o")
plt.plot(trace_df['elaps_time_us'], trace_df['rtt_us'], label = "direct-rtt", marker="o")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time (us)', fontsize=11)
plt.ylabel('rtt (us)', fontsize=11)
# plt.ylabel('cwnd (packets)', fontsize=11)
plt.savefig(plot_path+plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 