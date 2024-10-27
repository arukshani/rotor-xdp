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

def plot_vbuff(veth_df, plot_name):
    plt.plot(veth_df['elaps_time_us'], veth_df['buff_size'], label = "veth-node-2(veth-q)")
    plt.legend(fontsize=11)
    plt.xticks(fontsize=11)
    plt.yticks(fontsize=11)
    plt.xlabel('Time (us)', fontsize=11)
    plt.ylabel('Buffer Size (packets)', fontsize=11)
    plt.savefig(plot_name)

def plot_lbuff(local_df, plot_name):
    plt.plot(local_df['elaps_time_us'], local_df['buff_size'], label = "local-node-1(per-dest-q)")
    plt.legend(fontsize=11)
    plt.xticks(fontsize=11)
    plt.yticks(fontsize=11)
    plt.xlabel('Time (us)', fontsize=11)
    plt.ylabel('Buffer Size (packets)', fontsize=11)
    plt.savefig(plot_name)

def read_file(n1_file_name, local_q_num):
    n1_df = pd.read_csv(n1_file_name ,sep=',')
    n1_df = n1_df.loc[(n1_df['q_num'] == local_q_num)]

    pos = n1_df.columns.get_loc('time_ns')
    n1_df['elaps_time_ns'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
    n1_df['elaps_time_us'] =  n1_df['elaps_time_ns'] /1000
    n1_df.replace(np.nan, 0, inplace=True)

    return n1_df

exp_type = "opera"
path = "{}/exp-3-bbr/".format(exp_type)
plot_path = path+"/plots/"


# lbuff_file = "{}-lbuff-node-1.csv".format(exp_type)
# lbuff_plot_name = "{}-lbuff-node-1.png".format(exp_type)
# local_df = read_file(path+lbuff_file, 1)
# plot_lbuff(local_df, plot_path+lbuff_plot_name)

vbuff_file = "{}-vbuff-node-2.csv".format(exp_type)
vbuff_plot_name = "{}-vbuff-node-2.png".format(exp_type)
veth_df = read_file(path+vbuff_file, 0)
plot_vbuff(veth_df, plot_path+vbuff_plot_name)


