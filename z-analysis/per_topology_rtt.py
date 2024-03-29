
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

path = "/tmp/2024-03-25_16-52-58/"
path_to_cvs_n1=path+"all_packets_n1_rtt.csv"
path_to_cvs_n2=path+"all_packets_n2_rcvcontainer_time.csv"

node1_data = pd.read_csv(path+"afxdp-data-node-1.csv" ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
node1_data['node_name'] = "node-1"

node2_data = pd.read_csv(path+"afxdp-data-node-2.csv" ,sep=',', header=0,
        names=["node_ip", "slot", "topo_arr", "next_node", "time_ns", "time_part_sec", "time_part_nsec"])
node2_data['node_name'] = "node-2"

LINE_STYLES = ['solid', 'dashed', 'dashdot', 'dotted']
NUM_STYLES = len(LINE_STYLES)

def get_difference(rtt, ns):
    return rtt-ns

def plot_cdf_pure_latency_per_topo():
    cm = plt.get_cmap('gist_rainbow')
    rtt_summary = pd.read_csv(path_to_cvs_n1 ,sep=',', names=["send_topo", "recv_topo", "rtt", "next_node"])
    ns_summary = pd.read_csv(path_to_cvs_n2 ,sep=',', names=["send_topo", "recv_topo", "ns", "next_node"])
    # print(rtt_summary.count())
    # print(ns_summary.count())
    rtt_summary['pure_latency'] = np.vectorize(get_difference)(rtt_summary['rtt'], ns_summary['ns'])
    df_filter = rtt_summary[rtt_summary['send_topo'] == rtt_summary['recv_topo']]
    for num in range(1, 33):
        rtt_topo_x = df_filter.loc[(df_filter['send_topo'] == num)]
        # rtt_topo_x = rtt_topo_x.tail(100)
        count = rtt_topo_x['send_topo'].count()
        x = np.sort(rtt_topo_x['pure_latency'])
        y = np.arange(count) / float(count)
        # plt.figure().set_figwidth(100)
        # fig, ax = plt.subplots()
        cdf_p = plt.plot(x, y, label = "topo-{}".format(num), color=cm(num//3*3.0/32))
        cdf_p[0].set_linestyle(LINE_STYLES[num%NUM_STYLES])
    # plt.legend(loc="lower left", ncol=32)
    # h,l = ax.get_legend_handles_labels()
    plt.legend(ncol=3)
    plt.xlabel("[Pure Latency = RTT - Time Spent in Receiver's namespace] (us)")
    plt.ylabel('CDF')
    # plt.show()
    plt.savefig('per-topo-latency.png')
    plt.close()

def plot_cdf_pure_latency_all_topos():
    cm = plt.get_cmap('gist_rainbow')
    rtt_summary = pd.read_csv(path_to_cvs_n1 ,sep=',', names=["send_topo", "recv_topo", "rtt", "next_node"])
    ns_summary = pd.read_csv(path_to_cvs_n2 ,sep=',', names=["send_topo", "recv_topo", "ns", "next_node"])
    # print(rtt_summary.count())
    # print(ns_summary.count())
    rtt_summary['pure_latency'] = np.vectorize(get_difference)(rtt_summary['rtt'], ns_summary['ns'])
    df_filter = rtt_summary[rtt_summary['send_topo'] == rtt_summary['recv_topo']]
    x = np.sort(df_filter['pure_latency'])
    count = df_filter['send_topo'].count()
    y = np.arange(count) / float(count)
    plt.plot(x, y, marker='o')
    plt.legend()
    plt.xlabel("[Pure Latency = RTT - Time Spent in Receiver's namespace] (us)")
    plt.ylabel('CDF')
    # plt.show()
    plt.savefig('per-topo-latency.png')

def clean_up_n2():
    send_recv_mismatch = 0
    f = open(path_to_cvs_n2, 'w')
    writer = csv.writer(f)
    node2_data_last = node2_data.tail(12803)
    df_filter = node2_data_last[node2_data_last['slot'] != 1]
    for i in range(0,10000,2):
        index_1 = i
        row1=df_filter.iloc[index_1]
        index_2 = index_1+1
        row2=df_filter.iloc[index_2]
        if (row1['slot'] == 2 and row2['slot'] == 0):
            ns_us = (row2['time_ns'] - row1['time_ns'])/1000 #time spent in receivers namespace
            writer.writerow([row1['topo_arr'], row2['topo_arr'], ns_us, row1['next_node']])
        else:
            send_recv_mismatch = send_recv_mismatch+1
    f.close()
    print("There are {} rows with mismatch send-recv".format(send_recv_mismatch))

def clean_up_n1():
    send_recv_mismatch =0
    f = open(path_to_cvs_n1, 'w')
    writer = csv.writer(f)
    for i in range(0,10000,2):
        index_1 = i
        row1=node1_data.iloc[index_1]
        index_2 = index_1+1
        row2=node1_data.iloc[index_2]
        if (row1['slot'] == 0 and row2['slot'] == 2):
            rtt_us = (row2['time_ns'] - row1['time_ns'])/1000
            writer.writerow([row1['topo_arr'], row2['topo_arr'], rtt_us, row1['next_node']])
        else:
            print("Sending packet doesn't have an echo reponse")
            send_recv_mismatch=send_recv_mismatch+1
    f.close()
    print("There are {} rows with mismatch send & recv".format(send_recv_mismatch))


def main():
    # clean_up_n1()
    # clean_up_n2()
    # plot_cdf_pure_latency_all_topos()
    plot_cdf_pure_latency_per_topo()

if __name__ == '__main__':
    main()