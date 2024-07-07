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
plot_path = "multi-plots/plots/"

def get_df(file_name):
    trace_df = pd.read_csv(file_name ,sep=',')
    pos = trace_df.columns.get_loc('time')
    trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
    trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000
    return trace_df

plotname = "cwnd_compare.png"

direct_df = get_df(path+"4-cwnd-direct-node-1.csv")
sack_df = get_df(path+"2-cwnd-sack-node-1.csv")

plt.plot(sack_df['elaps_time_us'], sack_df['snd_cwnd'], label = "sack-opera", marker="o")
plt.plot(direct_df['elaps_time_us'], direct_df['snd_cwnd'], label = "direct", marker="o")


plt.legend(fontsize=12)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.xlabel('time (us)', fontsize=12)
plt.ylabel('cwnd (packets)', fontsize=12)
plt.savefig(plot_path+plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 