import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd
import plotly.graph_objs as go
import plotly
import plotly.express as px
import matplotlib.pyplot as plt
import re
import numpy as np
import seaborn as sns

def convert_to_us(value_something):
    return value_something * 1000

def read_file(path):
    rtt_df = pd.read_csv(path, header=0, names=["rtt_ms"])
    rtt_df['rtt_us'] = rtt_df.rtt_ms.apply(convert_to_us)
    return rtt_df

# direct_100us = read_file("/tmp/slot-selection/direct-slot-100us-ms.csv")
direct_200us = read_file("/tmp/slot-selection/direct-slot-200us-ms.csv")
# direct_1ms = read_file("/tmp/slot-selection/direct-slot-1ms-ms.csv")

opera_100us = read_file("/tmp/paper-latency-hop/opera-slot-100us-ms.csv")
opera_200us = read_file("/tmp/paper-latency-hop/opera-slot-200us-ms.csv")
opera_1ms = read_file("/tmp/paper-latency-hop/opera-slot-1ms-ms.csv")

print("100us max rtt {}".format(opera_100us['rtt_us'].max()))
print("200us max rtt {}".format(opera_200us['rtt_us'].max()))
print("1ms max rtt {}".format(opera_1ms['rtt_us'].max()))
# sns.kdeplot(data = direct_200us['rtt_us'], cumulative = True, label = "Direct-container-to-container")
# sns.kdeplot(data = opera_100us['rtt_us'], cumulative = True, label = "Opera-100μs-slot")
# sns.kdeplot(data = opera_200us['rtt_us'], cumulative = True, label = "Opera-200μs-slot")
# sns.kdeplot(data = opera_1ms['rtt_us'], cumulative = True, label = "Opera-1ms-slot")

fig, ax = plt.subplots()
sns.ecdfplot(data=direct_200us, x="rtt_us", ax=ax, label = "Direct-Container-to-Container")
sns.ecdfplot(data=opera_100us, x="rtt_us", ax=ax, label = "Opera-100μs-Slot")
sns.ecdfplot(data=opera_200us, x="rtt_us", ax=ax, label = "Opera-200μs-Slot")
sns.ecdfplot(data=opera_1ms, x="rtt_us", ax=ax, label = "Opera-1ms-Slot")

y_special = 0.95
for line in ax.get_lines():
    x, y = line.get_data()
    ind = np.argwhere(y >= y_special)[0, 0]  # first index where y is larger than y_special
    # x[ind] is the desired x-value
    # ax.text(x[ind], y_special, f' {x[ind]:.1f}', ha='right', va='top') # maybe color=line.get_color()
    if(x[ind] == 41.0):
        ax.text(x[ind], y_special, f' {x[ind]:.0f}', ha='right', va='top', color=line.get_color(), weight='bold', fontsize=10) # maybe color=line.get_color()
    if(x[ind] == 129.0):
        ax.text(x[ind], y_special, f' {x[ind]:.0f}', ha='left', va='top', color=line.get_color(), weight='bold', fontsize=10) # maybe color=line.get_color()
    # if(x[ind] == 116.0):
    #     ax.text(x[ind], y_special, f' {x[ind]:.0f}', ha='right', va='top', color=line.get_color(), weight='bold', fontsize=10) # maybe color=line.get_color()
    if(x[ind] == 107.0):
        ax.text(x[ind], y_special, f' {x[ind]:.0f}', ha='right', va='bottom', color=line.get_color(), weight='bold', fontsize=10) # maybe color=line.get_color()
    print(x[ind])
ax.axhline(y_special, linestyle='--', color='#cfcfcf', lw=2, alpha=0.75)

plt.legend(fontsize=14)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('RTT (μs)', fontsize=14)
plt.ylabel('CDF', fontsize=14)
# plt.savefig('P-ALL-RTTs.png')
plt.savefig('P-ALL-RTTs.pdf')

## HTML plot
# hist, bin_edges = np.histogram(direct_200us['rtt_us'], bins=100, density=True)
# cdf = np.cumsum(hist * np.diff(bin_edges))

# hist_1, bin_edges_1 = np.histogram(opera_100us['rtt_us'], bins=100, density=True)
# cdf_1 = np.cumsum(hist_1 * np.diff(bin_edges_1))

# hist_2, bin_edges_2 = np.histogram(opera_200us['rtt_us'], bins=100, density=True)
# cdf_2 = np.cumsum(hist_2 * np.diff(bin_edges_2))

# hist_3, bin_edges_3 = np.histogram(opera_1ms['rtt_us'], bins=100, density=True)
# cdf_3 = np.cumsum(hist_3 * np.diff(bin_edges_3))

# fig = go.Figure(data=[
#     go.Scatter(x=bin_edges, y=cdf, name='Direct Ping RTT'),
#     go.Scatter(x=bin_edges_1, y=cdf_1, name='Opera-100us-slot'),
#     go.Scatter(x=bin_edges_2, y=cdf_2, name='Opera-200us-slot'),
#     go.Scatter(x=bin_edges_3, y=cdf_3, name='Opera-1ms-slot'),
# ])
# # fig = go.Figure()
# fig.write_html("all-slot-hop-rtt.html")