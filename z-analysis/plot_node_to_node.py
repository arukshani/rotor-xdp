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

def get_total_delay(fwd_delay, return_delay):
    return (fwd_delay+return_delay)

def get_rtt(recv_time_part_sec, send_time_part_sec, recv_time_part_nsec, send_time_part_nsec):
    
    rtt_sec = int(recv_time_part_sec) - int(send_time_part_sec)
    rtt_nec = int(recv_time_part_nsec) - int(send_time_part_nsec)
    if ('rtt_sec' != 0):
        rtt_nec = rtt_nec + (rtt_sec * 1000000000)
        return rtt_nec/1000
    else:
        return rtt_nec/1000

######################################UDP RTT######################################
path = "/tmp/NODE-to-NODE/"
udp_rtt_file = "udp_client_rtt.csv"

## seq_id,send_time_part_sec,send_time_part_nsec,recv_time_part_sec,recv_time_part_nsec
udp_rtt = pd.read_csv(path+udp_rtt_file ,sep=',')
## remove first row
udp_rtt = udp_rtt.iloc[1:]
udp_rtt['rtt_us'] = udp_rtt.apply(lambda row: get_rtt(row['recv_time_part_sec'], row['send_time_part_sec'],
                                                            row['recv_time_part_nsec'], row['send_time_part_nsec']), axis=1)

######################################END OF UDP RTT######################################

######################################AF_XDP DELAY######################################

node_1_file = "2024-04-18_15-18-56/afxdp-data-node-1.csv"
node_2_file = "2024-04-18_15-18-56/afxdp-data-node-2.csv"
## node_ip,slot,topo_arr,next_node,time_ns,time_part_sec,time_part_nsec

node_1 = pd.read_csv(path+node_1_file ,sep=',')
node_2 = pd.read_csv(path+node_2_file ,sep=',')

n1_tx_df = node_1.loc[(node_1['slot'] == 0)]
n2_rx_df = node_2.loc[(node_2['slot'] == 2)]
n1_tx_df.reset_index(inplace=True)
n2_rx_df.reset_index(inplace=True)
n1_tx_df = n1_tx_df.rename(columns={'slot': 'send_slot', 
                                    'time_part_sec': 'send_sec', 'time_part_nsec': 'send_nsec'})
n2_rx_df = n2_rx_df.rename(columns={'slot': 'recv_slot', 
                                    'time_part_sec': 'recv_sec', 'time_part_nsec': 'recv_nsec'})
fwd_afxdp = pd.concat([n1_tx_df, n2_rx_df], axis=1)
fwd_afxdp['fwd_delay_us'] = fwd_afxdp.apply(lambda row: get_rtt(row['recv_sec'], row['send_sec'],
                                                            row['recv_nsec'], row['send_nsec']), axis=1)

# print(fwd_afxdp[(fwd_afxdp['fwd_delay_us'] < 0)])
# fwd_afxdp.to_csv("fwd.csv", sep=',')
# print(n1_tx_df.head(5))
# print(n2_rx_df.head(5))
# print(fwd_afxdp.head(5))
n1_rx_df = node_1.loc[(node_1['slot'] == 2)]
n2_tx_df = node_2.loc[(node_2['slot'] == 0)]
n1_rx_df.reset_index(inplace=True)
n2_tx_df.reset_index(inplace=True)
n2_tx_df = n2_tx_df.rename(columns={'slot': 'send_slot', 
                                    'time_part_sec': 'send_sec', 'time_part_nsec': 'send_nsec'})
n1_rx_df = n1_rx_df.rename(columns={'slot': 'recv_slot', 
                                    'time_part_sec': 'recv_sec', 'time_part_nsec': 'recv_nsec'})
return_afxdp = pd.concat([n2_tx_df, n1_rx_df], axis=1)
return_afxdp['return_delay_us'] = return_afxdp.apply(lambda row: get_rtt(row['recv_sec'], row['send_sec'],
                                                            row['recv_nsec'], row['send_nsec']), axis=1)

# print(return_afxdp[(return_afxdp['return_delay_us'] < 0)])

fwd_afxdp.reset_index(inplace=True)
return_afxdp.reset_index(inplace=True)
af_xdp_delay_summary = pd.concat([fwd_afxdp['fwd_delay_us'], return_afxdp['return_delay_us']], axis=1)
af_xdp_delay_summary['total_delay_us'] = af_xdp_delay_summary.apply(lambda row: get_total_delay(row['fwd_delay_us'], row['return_delay_us']), axis=1)
# print(fwd_afxdp.head(5))
# print(return_afxdp.head(5))
# print(af_xdp_delay_summary.head(5))
######################################END OF AF_XDP DELAY######################################
### PLOT
# sns.kdeplot(data = udp_rtt['rtt_us'], cumulative = True, label = "udp-rtt")
# sns.kdeplot(data = fwd_afxdp['fwd_delay_us'], cumulative = True, label = "afxdp_fwd")
# sns.kdeplot(data = return_afxdp['return_delay_us'], cumulative = True, label = "afxdp_return")
# sns.kdeplot(data = af_xdp_delay_summary['total_delay_us'], cumulative = True, label = "tot_afxdp_delay")
# plt.legend()
# plt.xlabel('Delay (us)')
# plt.ylabel('CDF')
# plt.savefig('af_xdp_one_way.png')

hist, bin_edges = np.histogram(udp_rtt['rtt_us'], bins=100, density=True)
cdf = np.cumsum(hist * np.diff(bin_edges))

hist_1, bin_edges_1 = np.histogram(fwd_afxdp['fwd_delay_us'], bins=100, density=True)
cdf_1 = np.cumsum(hist_1 * np.diff(bin_edges_1))

hist_2, bin_edges_2 = np.histogram(return_afxdp['return_delay_us'], bins=100, density=True)
cdf_2 = np.cumsum(hist_2 * np.diff(bin_edges_2))

hist_3, bin_edges_3 = np.histogram(af_xdp_delay_summary['total_delay_us'], bins=100, density=True)
cdf_3 = np.cumsum(hist_3 * np.diff(bin_edges_3))

fig = go.Figure(data=[
    go.Scatter(x=bin_edges, y=cdf, name='UDP RTT'),
    go.Scatter(x=bin_edges_1, y=cdf_1, name='AFXDP FWD DELAY'),
    go.Scatter(x=bin_edges_2, y=cdf_2, name='AFXDP RETURN DELAY'),
    go.Scatter(x=bin_edges_3, y=cdf_3, name='AFXDP TOTAL DELAY'),
])
# fig = go.Figure()
# fig.add_traces(go.Scatter(x=topo_results['packet_time'], y = topo_results['packets'], mode = 'lines', name=legend_str))
fig.write_html("udp_results.html")