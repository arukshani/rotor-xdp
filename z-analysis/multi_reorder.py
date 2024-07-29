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


base_path = "iperf-cubic-7/"
opera_path = base_path+"opera-4000/"
plotname = "opera_reorder_compare_4000.png"

direct_path = base_path+"direct-4000/"
# plotname = "d1_vs_o5_4000.png"

def get_df(file_name):
    trace_df = pd.read_csv(file_name ,sep=',')
    pos = trace_df.columns.get_loc('time')
    trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
    trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000

    # mask = (trace_df['elaps_time_us'] > 0) & (trace_df['elaps_time_us'] <= 10000000)
    # trace_df = trace_df.loc[mask]

    return trace_df

direct_df = get_df(direct_path+"exp-1/direct-ss-node-1.csv")

opera_df1 = get_df(opera_path+"exp-1/opera-ss-node-1.csv")
opera_df2 = get_df(opera_path+"exp-2/opera-ss-node-1.csv")
opera_df3 = get_df(opera_path+"exp-3/opera-ss-node-1.csv")
opera_df4 = get_df(opera_path+"exp-4/opera-ss-node-1.csv")
opera_df5 = get_df(opera_path+"exp-5/opera-ss-node-1.csv")

plt.plot(direct_df['elaps_time_us'], direct_df['reordering'], label = "direct-1")

plt.plot(opera_df1['elaps_time_us'], opera_df1['reordering'], label = "opera-1")
plt.plot(opera_df2['elaps_time_us'], opera_df2['reordering'], label = "opera-2")
plt.plot(opera_df3['elaps_time_us'], opera_df3['reordering'], label = "opera-3")
plt.plot(opera_df4['elaps_time_us'], opera_df4['reordering'], label = "opera-4")
plt.plot(opera_df5['elaps_time_us'], opera_df5['reordering'], label = "opera-5")

plt.legend(fontsize=10, loc = "lower center",)
# plt.legend(fontsize=12)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.xlabel('time (us)', fontsize=12)
plt.ylabel('reordering distance (packets)', fontsize=12)
plt.savefig(base_path+plotname)

 