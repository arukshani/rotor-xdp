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

def plot_rtt(trace_df, plot_name):
    plt.plot(trace_df['elaps_time_us'], trace_df['rtt_us'], label = "rtt")
    plt.legend(fontsize=11)
    plt.xticks(fontsize=11)
    plt.yticks(fontsize=11)
    plt.xlabel('Time (us)', fontsize=11)
    plt.ylabel('RTT (us)', fontsize=11)
    plt.savefig(plot_name)

def plot_cwnd_and_ssthresh(trace_df, plot_name):
    plt.plot(trace_df['elaps_time_us'], trace_df['snd_cwnd'], label = "cwnd")
    plt.plot(trace_df['elaps_time_us'], trace_df['ssthresh'], label = "ssthresh", linestyle='dashed')
    plt.legend(fontsize=11)
    plt.xticks(fontsize=11)
    plt.yticks(fontsize=11)
    plt.xlabel('Time (us)', fontsize=11)
    plt.ylabel('Packets', fontsize=11)
    plt.savefig(plot_name)
    # plt.savefig('P-ALL-RTTs.pdf')

def get_df(file_path, from_time, to_time):
    trace_df = pd.read_csv(file_path ,sep=',')
    pos = trace_df.columns.get_loc('time')
    trace_df['elaps_time'] =  trace_df.iloc[1:, pos] - trace_df.iat[0, pos]
    trace_df['elaps_time_us'] =  trace_df['elaps_time'] * 1000000

    if (to_time != 0):
        mask = (trace_df['elaps_time_us'] > from_time) & (trace_df['elaps_time_us'] <= to_time)
        trace_df = trace_df.loc[mask]
    return trace_df


exp_type = "opera"
path = "{}/exp-2/".format(exp_type)
trace_file = "{}-ss-node-1.csv".format(exp_type)
plot_path = path+"/plots/"

from_time = 0
to_time = 0
to_time = 1000000

if (to_time == 0):
    cwnd_plot_name = "{}-cwnd-node-1.png".format(exp_type)
else:
    cwnd_plot_name = "{}-cwnd-t{}-{}-node-1.png".format(exp_type, from_time, to_time)

trace_df = get_df(path+trace_file,from_time,to_time)
plot_cwnd_and_ssthresh(trace_df, plot_path+cwnd_plot_name)
# plot_rtt(trace_df, plot_path+"opera-rtt-node-1.png")




 