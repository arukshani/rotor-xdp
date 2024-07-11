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

path = "throughput/multi-plots/"
plotname = path+"srtt-all.png"

direct_file = "2-direct-cwnd-node-1.csv"
opera_file = "3-opera-cwnd-node-1.csv"

direct_df = pd.read_csv(path+direct_file ,sep=',')
dir_pos = direct_df.columns.get_loc('time')
direct_df['elaps_time'] =  direct_df.iloc[1:, dir_pos] - direct_df.iat[0, dir_pos]
direct_df['elaps_time_us'] =  direct_df['elaps_time'] * 1000000

opera_df = pd.read_csv(path+opera_file ,sep=',')
opera_pos = opera_df.columns.get_loc('time')
opera_df['elaps_time'] =  opera_df.iloc[1:, opera_pos] - opera_df.iat[0, opera_pos]
opera_df['elaps_time_us'] =  opera_df['elaps_time'] * 1000000

# plt.plot(trace_df['time'], trace_df['srtt'], label = "srtt")
plt.plot(direct_df['elaps_time_us'], direct_df['srtt'], label = "direct")
plt.plot(opera_df['elaps_time_us'], opera_df['srtt'], label = "opera")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time', fontsize=11)
plt.ylabel('srtt', fontsize=11)
# plt.ylabel('srtt (us)', fontsize=11)
plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 