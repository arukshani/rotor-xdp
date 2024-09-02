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


# base_path = "iperf-cubic-default/"
opera_path = "opera/"
plotname = "cwnd_compare_11-1s.png"

direct_path = "direct/"
# plotname = "d1_vs_o5_ss_2s.png"


def get_df(file_name):
    trace_df = pd.read_csv(file_name ,sep=',')
    pos = trace_df.columns.get_loc('time')
    trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
    trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000

    mask = (trace_df['elaps_time_us'] > 0) & (trace_df['elaps_time_us'] <= 1000000)
    trace_df = trace_df.loc[mask]

    return trace_df

direct_df1 = get_df(direct_path+"exp-1/direct-ss-node-1.csv")
# direct_df2 = get_df(direct_path+"exp-2/direct-ss-node-1.csv")
# direct_df3 = get_df(direct_path+"exp-3/direct-ss-node-1.csv")
# direct_df4 = get_df(direct_path+"exp-4/direct-ss-node-1.csv")
# direct_df5 = get_df(direct_path+"exp-5/direct-ss-node-1.csv")

opera_df1 = get_df(opera_path+"exp-1/opera-ss-node-1.csv")
# opera_df2 = get_df(opera_path+"exp-2/opera-ss-node-1.csv")
# opera_df3 = get_df(opera_path+"exp-3/opera-ss-node-1.csv")
# opera_df4 = get_df(opera_path+"exp-4/opera-ss-node-1.csv")
# opera_df5 = get_df(opera_path+"exp-5/opera-ss-node-1.csv")

plt.plot(direct_df1['elaps_time_us'], direct_df1['snd_cwnd'], label = "direct-1-cwnd")
# plt.plot(direct_df1['elaps_time_us'], direct_df1['ssthresh'], label = "direct-1-ssthresh", linestyle='dashed')
# plt.plot(direct_df2['elaps_time_us'], direct_df2['snd_cwnd'], label = "direct-2")
# plt.plot(direct_df3['elaps_time_us'], direct_df3['snd_cwnd'], label = "direct-3")
# plt.plot(direct_df4['elaps_time_us'], direct_df4['snd_cwnd'], label = "direct-4")
# plt.plot(direct_df5['elaps_time_us'], direct_df5['snd_cwnd'], label = "direct-5")

plt.plot(opera_df1['elaps_time_us'], opera_df1['snd_cwnd'], label = "opera-1-cwnd")
# plt.plot(opera_df1['elaps_time_us'], opera_df1['ssthresh'], label = "opera-1-ssthresh", linestyle='dashed')
# plt.plot(opera_df2['elaps_time_us'], opera_df2['snd_cwnd'], label = "opera-2")
# plt.plot(opera_df2['elaps_time_us'], opera_df2['ssthresh'], label = "opera-2-ssthresh", linestyle='dashed')
# plt.plot(opera_df3['elaps_time_us'], opera_df3['snd_cwnd'], label = "opera-3")
# plt.plot(opera_df3['elaps_time_us'], opera_df3['ssthresh'], label = "opera-3-ssthresh", linestyle='dashed')
# plt.plot(opera_df4['elaps_time_us'], opera_df4['snd_cwnd'], label = "opera-4")
# plt.plot(opera_df4['elaps_time_us'], opera_df4['ssthresh'], label = "opera-4-ssthresh", linestyle='dashed')
# plt.plot(opera_df5['elaps_time_us'], opera_df5['snd_cwnd'], label = "opera-5")
# plt.plot(opera_df5['elaps_time_us'], opera_df5['ssthresh'], label = "opera-5-ssthresh", linestyle='dashed')


# plt.legend(fontsize=7, loc = "lower right", bbox_to_anchor=(0.1,1))
plt.legend(fontsize=7)
plt.xticks(fontsize=12)
plt.yticks(fontsize=12)
plt.xlabel('time (us)', fontsize=12)
plt.ylabel('cwnd (packets)', fontsize=12)
plt.savefig("opera/exp-1/plots/"+plotname)

 