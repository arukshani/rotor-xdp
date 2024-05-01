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

def get_tot(fwd, ret):
    return (fwd+ret)

path = "/tmp/HOPS/"

node_1_file = "2024-04-19_15-42-32/afxdp-data-node-1.csv"
node_2_file = "2024-04-19_15-42-32/afxdp-data-node-2.csv"

## node_ip,slot,topo_arr,next_node,time_ns,time_part_sec,time_part_nsec,hop_count

node_1 = pd.read_csv(path+node_1_file ,sep=',')
node_2 = pd.read_csv(path+node_2_file ,sep=',')

fwd_hops = node_2.loc[(node_2['slot'] == 2)]
ret_hops = node_1.loc[(node_1['slot'] == 2)]

# print(fwd_hops[(fwd_hops['hop_count'] > 3)])
# print(ret_hops[(ret_hops['hop_count'] > 4)])

sns.kdeplot(data = fwd_hops['hop_count'], cumulative = True, label = "fwd-path-hop-count")
sns.kdeplot(data = ret_hops['hop_count'], cumulative = True, label = "return-path-hop-count")

fwd_hops.reset_index(inplace=True)
ret_hops.reset_index(inplace=True)

# print(fwd_hops.head(5))
# print(ret_hops.head(5))

fwd_hops = fwd_hops.rename(columns={'hop_count': 'fwd_hop_count'})
ret_hops = ret_hops.rename(columns={'hop_count': 'return_hop_count'})
tot_hops = pd.concat([fwd_hops, ret_hops], axis=1)

tot_hops['tot_hop_count'] = tot_hops.apply(lambda row: get_tot(row['fwd_hop_count'], row['return_hop_count']), axis=1)

# print(tot_hops.head(5))

# print(tot_hops[(tot_hops['tot_hop_count'] > 6)])

sns.kdeplot(data = tot_hops['tot_hop_count'], cumulative = True, label = "total-hop-count")
plt.legend()
plt.xlabel('Hop Count', fontsize=16)
plt.ylabel('CDF', fontsize=16)
plt.savefig('hop-count.pdf')

 