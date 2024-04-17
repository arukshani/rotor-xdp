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

path = "/tmp/XDP/2024-04-17_13-30-47/"
node1_file = "afxdp-data-node-1.csv"

def get_timestamp(time_format):
    return time_format.timestamp() * 1000 * 1000

def plot_sent_data(node1_data, fig):
    for x in range(1, 33):
        topo_data = node1_data[node1_data['topo_arr'] == x]
        topo_results  = topo_data.resample('100us', on='packet_time')['packets'].sum().dropna().reset_index()
        legend_str = "n1-sent-topo-" + str(x)
        fig.add_traces(go.Scatter(x=topo_results['packet_time'], y = topo_results['packets'], mode = 'lines', name=legend_str))

def plot_recv_data(fig):
    # print("hello 1")
    for x in range(2, 32):
        filename = "afxdp-data-node-" + str(x) + ".csv"
        rcv_data = pd.read_csv(path+filename ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
        rcv_data['node_name'] = "node-" + str(x)
        rcv_data['time_convert'] = pd.to_datetime(rcv_data['time_ns'], infer_datetime_format=True)
        rcv_data['packet_time'] = pd.to_datetime(rcv_data.time_convert, unit='us')
        rcv_data['timestamp']= rcv_data.packet_time.apply(get_timestamp)
        rcv_data['packets']= 1
        # print("hello \n")
        # print(rcv_data.head(10))
        slot1_data = rcv_data[rcv_data['slot'] == 1]
        recv_results = slot1_data.resample('100us', on='packet_time')['packets'].sum().dropna().reset_index()
        legend_str = "n" + str(x)+ "-rcv"
        fig.add_traces(go.Scatter(x=recv_results['packet_time'], y = recv_results['packets'], mode = 'lines', name=legend_str))

node1_data = pd.read_csv(path+"afxdp-data-node-1.csv" ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
node1_data['node_name'] = "node-1"
node1_data['time_convert'] = pd.to_datetime(node1_data['time_ns'], infer_datetime_format=True)
node1_data['packet_time'] = pd.to_datetime(node1_data.time_convert, unit='us')
node1_data['timestamp']= node1_data.packet_time.apply(get_timestamp)
node1_data['packets']= 1
# node1_results = node1_data.resample('100us', on='packet_time')['packets'].sum().dropna().reset_index()
n1_slot1 = node1_data[node1_data['slot'] == 0]
fig = go.Figure()

plot_sent_data(n1_slot1, fig)
plot_recv_data(fig)

# fig.add_traces(go.Scatter(x=node1_results['packet_time'], y = node1_results['packets'], mode = 'lines', name="node-1"))
# # fig.add_traces(go.Scatter(x=node1_data['timestamp'], y = node1_data['topo_arr'], mode = 'lines', name="node-1"))
# fig.write_image("topo-plot.jpg")
fig.write_html("ptp_test.html")