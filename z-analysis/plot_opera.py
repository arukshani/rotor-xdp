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
# path = "/tmp/NODE-to-NODE/"
# udp_rtt_file = "udp_client_rtt.csv"

# ## seq_id,send_time_part_sec,send_time_part_nsec,recv_time_part_sec,recv_time_part_nsec
# udp_rtt = pd.read_csv(path+udp_rtt_file ,sep=',')
# ## remove first row
# udp_rtt = udp_rtt.iloc[1:]
# udp_rtt['rtt_us'] = udp_rtt.apply(lambda row: get_rtt(row['recv_time_part_sec'], row['send_time_part_sec'],
#                                                             row['recv_time_part_nsec'], row['send_time_part_nsec']), axis=1)

######################################END OF UDP RTT######################################
path_op = "/tmp/TAIL/"
udp_rtt_file_op = "opera_udp_client_rtt.csv"

## seq_id,send_time_part_sec,send_time_part_nsec,recv_time_part_sec,recv_time_part_nsec
opera_udp_rtt = pd.read_csv(path_op+udp_rtt_file_op ,sep=',')
## remove first row
opera_udp_rtt = opera_udp_rtt.iloc[1:]
opera_udp_rtt['rtt_us'] = opera_udp_rtt.apply(lambda row: get_rtt(row['recv_time_part_sec'], row['send_time_part_sec'],
                                                            row['recv_time_part_nsec'], row['send_time_part_nsec']), axis=1)

# print(opera_udp_rtt[(opera_udp_rtt['rtt_us'] > 150)])

### PLOT
# sns.kdeplot(data = udp_rtt['rtt_us'], cumulative = True, label = "udp-rtt")
sns.kdeplot(data = opera_udp_rtt['rtt_us'], cumulative = True, label = "opera-udp-rtt")
plt.legend()
plt.xlabel('RTT (us)')
plt.ylabel('CDF')
plt.savefig('opera_udp.png')

# hist, bin_edges = np.histogram(udp_rtt['rtt_us'], bins=100, density=True)
# cdf = np.cumsum(hist * np.diff(bin_edges))

# hist_1, bin_edges_1 = np.histogram(opera_udp_rtt['rtt_us'], bins=100, density=True)
# cdf_1 = np.cumsum(hist_1 * np.diff(bin_edges_1))


# fig = go.Figure(data=[
#     go.Scatter(x=bin_edges, y=cdf, name='UDP RTT'),
#     go.Scatter(x=bin_edges_1, y=cdf_1, name='OPERA UDP RTT')
# ])
# fig.write_html("opera_udp_results.html")