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

path = "sack-small-file/"
plot_path = "sack-small-file/plots/"

trace_file = "2-cwnd-sack-node-1.csv"
plotname = "2-cwnd-sack-node-1.png"

trace_df = pd.read_csv(path+trace_file ,sep=',')
pos = trace_df.columns.get_loc('time')
trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000
print(trace_df.head(5))

plt.plot(trace_df['elaps_time_us'], trace_df['snd_cwnd'], label = "snd_cwnd", marker="o")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=14)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('time (us)', fontsize=14)
plt.ylabel('cwnd (packets)', fontsize=14)
plt.savefig(plot_path+plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 