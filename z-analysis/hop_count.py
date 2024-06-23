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

path = "/tmp/slot-selection-hops/"

def read_file(file_name):
    node_df = pd.read_csv(path+file_name ,sep=',')
    return node_df

n1_df_100us = read_file("n1-100us-log.csv")
n2_df_100us = read_file("n2-100us-log.csv")
n1_df_200us = read_file("n1-200us-log.csv")
n2_df_200us = read_file("n2-200us-log.csv")
n1_df_1ms = read_file("n1-1ms-log.csv")
n2_df_1ms= read_file("n2-1ms-log.csv")

fwd_hops_100us = n2_df_100us.loc[(n2_df_100us['slot'] == 2)]
ret_hops_100us = n1_df_100us.loc[(n1_df_100us['slot'] == 2)]
fwd_hops_200us = n2_df_200us.loc[(n2_df_200us['slot'] == 2)]
ret_hops_200us = n1_df_200us.loc[(n1_df_200us['slot'] == 2)]
fwd_hops_1ms = n2_df_1ms.loc[(n2_df_1ms['slot'] == 2)]
ret_hops_1ms = n1_df_1ms.loc[(n1_df_1ms['slot'] == 2)]

fig, ax = plt.subplots()

fwd_hops_100us.reset_index(inplace=True)
ret_hops_100us.reset_index(inplace=True)
fwd_hops_100us = fwd_hops_100us.rename(columns={'hop_count': 'fwd_hop_count'})
ret_hops_100us = ret_hops_100us.rename(columns={'hop_count': 'return_hop_count'})
tot_hops_100us = pd.concat([fwd_hops_100us, ret_hops_100us], axis=1)
tot_hops_100us['tot_hop_count'] = tot_hops_100us.apply(lambda row: get_tot(row['fwd_hop_count'], row['return_hop_count']), axis=1)
# sns.kdeplot(data = tot_hops_100us['tot_hop_count'], cumulative = True, label = "100μs-Slot")
sns.ecdfplot(data=tot_hops_100us, x="tot_hop_count", ax=ax, label = "100μs-Slot")

fwd_hops_200us.reset_index(inplace=True)
ret_hops_200us.reset_index(inplace=True)
fwd_hops_200us = fwd_hops_200us.rename(columns={'hop_count': 'fwd_hop_count'})
ret_hops_200us = ret_hops_200us.rename(columns={'hop_count': 'return_hop_count'})
tot_hops_200us = pd.concat([fwd_hops_200us, ret_hops_200us], axis=1)
tot_hops_200us['tot_hop_count'] = tot_hops_200us.apply(lambda row: get_tot(row['fwd_hop_count'], row['return_hop_count']), axis=1)
# sns.kdeplot(data = tot_hops_200us['tot_hop_count'], cumulative = True, label = "200μs-Slot")
sns.ecdfplot(data=tot_hops_200us, x="tot_hop_count", ax=ax, label = "200μs-Slot")

fwd_hops_1ms.reset_index(inplace=True)
ret_hops_1ms.reset_index(inplace=True)
fwd_hops_1ms = fwd_hops_1ms.rename(columns={'hop_count': 'fwd_hop_count'})
ret_hops_1ms = ret_hops_1ms.rename(columns={'hop_count': 'return_hop_count'})
tot_hops_1ms = pd.concat([fwd_hops_1ms, ret_hops_1ms], axis=1)
tot_hops_1ms['tot_hop_count'] = tot_hops_1ms.apply(lambda row: get_tot(row['fwd_hop_count'], row['return_hop_count']), axis=1)
# sns.kdeplot(data = tot_hops_1ms['tot_hop_count'], cumulative = True, label = "1ms-Slot")
sns.ecdfplot(data=tot_hops_1ms, x="tot_hop_count", ax=ax, label = "1ms-Slot")

# node_1_file = "n1-100us-log.csv"
# node_2_file = "n2-100us-log.csv"

## slot,topo_arr,time_ns,time_part_sec,time_part_nsec,hop_count

# node_1 = pd.read_csv(path+node_1_file ,sep=',')
# node_2 = pd.read_csv(path+node_2_file ,sep=',')

# fwd_hops = node_2.loc[(node_2['slot'] == 2)]
# ret_hops = node_1.loc[(node_1['slot'] == 2)]

# print(fwd_hops[(fwd_hops['hop_count'] > 3)])
# print(ret_hops[(ret_hops['hop_count'] > 4)])

# sns.kdeplot(data = fwd_hops['hop_count'], cumulative = True, label = "fwd-path-hop-count")
# sns.kdeplot(data = ret_hops['hop_count'], cumulative = True, label = "return-path-hop-count")

# fwd_hops.reset_index(inplace=True)
# ret_hops.reset_index(inplace=True)

# print(fwd_hops.head(5))
# print(ret_hops.head(5))

# fwd_hops = fwd_hops.rename(columns={'hop_count': 'fwd_hop_count'})
# ret_hops = ret_hops.rename(columns={'hop_count': 'return_hop_count'})
# tot_hops = pd.concat([fwd_hops, ret_hops], axis=1)

# tot_hops['tot_hop_count'] = tot_hops.apply(lambda row: get_tot(row['fwd_hop_count'], row['return_hop_count']), axis=1)

# print(tot_hops.head(5))

# print(tot_hops[(tot_hops['tot_hop_count'] > 6)])

# sns.kdeplot(data = tot_hops['tot_hop_count'], cumulative = True, label = "total-hop-count")

# sns.set(font_scale=1.1)
plt.legend(fontsize=14)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('Hop Counts for ping RTTs', fontsize=16)
plt.ylabel('CDF', fontsize=16)
# plt.savefig('P-HopCountDist.pdf')
plt.savefig('all-hop-count.png')

 