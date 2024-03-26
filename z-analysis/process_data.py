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

def get_latency_(recv_time_part_sec, send_time_part_sec, recv_time_part_nsec, send_time_part_nsec):
    
    rtt_sec = int(recv_time_part_sec) - int(send_time_part_sec)
    rtt_nec = int(recv_time_part_nsec) - int(send_time_part_nsec)
    if ('rtt_sec' != 0):
        rtt_nec = rtt_nec + (rtt_sec * 1000000000)
        return rtt_nec
    else:
        return rtt_nec

def get_latency(recv, send):
    latency_ns = int(recv) - int(send)
    return latency_ns

path = "/tmp/2024-03-25_16-52-58/"

node1_data = pd.read_csv(path+"afxdp-data-node-1.csv" ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
node1_data['node_name'] = "node-1"

node2_data = pd.read_csv(path+"afxdp-data-node-2.csv" ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
node2_data['node_name'] = "node-2"

# print(node1_data.head(10))
# print(node2_data.head(10))

node1_send = node1_data.loc[(node1_data['slot'] == 0)]
node2_recv = node2_data.loc[(node2_data['slot'] == 2)]

node1_send.rename(columns={'node_ip': 'node_ip_1', 
                'slot': 'slot_1',
                'topo_arr': 'topo_arr_1',
                'next_node': 'next_node_1',
                'time_ns': 'time_ns_1',
                'time_part_sec': 'time_part_sec_1',
                'time_part_nsec': 'time_part_nsec_1'
                }, 
                inplace=True)

node2_recv.rename(columns={'node_ip': 'node_ip_2', 
                'slot': 'slot_2',
                'topo_arr': 'topo_arr_2',
                'next_node': 'next_node_2',
                'time_ns': 'time_ns_2',
                'time_part_sec': 'time_part_sec_2',
                'time_part_nsec': 'time_part_nsec_2'
                }, 
                inplace=True)

node1_send.reset_index(drop=True, inplace=True)
node2_recv.reset_index(drop=True, inplace=True)

# print(len(node1_send[node1_send['time_ns_1'].isna()]))
# print(len(node2_recv[node2_recv['time_ns_2'].isna()]))
# print(node1_send.head(10))
# print(node2_recv.head(10))

# print(len(node1_send))
# print(len(node1_send))


##Calulate forward latency
fwd_afxdp = pd.concat([node1_send, node2_recv], axis=1)

print(len(fwd_afxdp[fwd_afxdp['time_ns_2'].isna()]))


fwd_afxdp['fwd_delay_nsec'] = fwd_afxdp.apply(lambda row: get_latency(row['time_ns_2'], 
                                                    row['time_ns_1']), axis=1)

# fwd_afxdp['fwd_delay_nsec'] = fwd_afxdp.apply(lambda row: get_latency_(row['time_part_sec_2'], row['time_part_sec_1'],
#                                                             row['time_part_nsec_2'], row['time_part_nsec_1']), axis=1)
            

# print(fwd_afxdp.head(10))

fwd_afxdp.to_csv(path+"afxdp_fwd_delay.csv", sep=',')

##Calulate return latency
