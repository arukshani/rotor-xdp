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

path = "data/seq/"

def read_file(n1_file_name):
    n1_df = pd.read_csv(path+n1_file_name ,sep=',')
    return n1_df

n1_df = read_file("direct-node-1.csv")
n1_df = n1_df.loc[(n1_df['slot'] == 0)]
# print(n1_df.head(15))

pos = n1_df.columns.get_loc('time_ns')
n1_df['elaps_time_ns'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
n1_df['elaps_time_us'] =  n1_df['elaps_time_ns'] /1000
n1_df.replace(np.nan, 0, inplace=True)
# print(n1_df.head(100))
n1_df = n1_df.head(2000)

print(n1_df[['seq', 'topo_arr', 'elaps_time_us']].head(10))

# periods = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32]
periods = n1_df[n1_df.topo_arr.diff()!=0].elaps_time_us
print(periods)

plt.plot(n1_df['elaps_time_us'], n1_df['seq'], label = "direct")
# for item in periods[1::]:
for item in periods:
    plt.axvline(item, ymin=0, ymax=1,color='red')

# ax = sns.lineplot(x="elaps_time_us", y="seq", data=n1_df)
# ax.axvspan(3, 100,facecolor="darkmagenta", edgecolor='black', hatch="o", alpha=.3, label="Easter week")

plt.legend(fontsize=14)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('time (us)', fontsize=14)
plt.ylabel('sequence number', fontsize=14)
plt.savefig('seq-direct.png')
# plt.savefig('P-ALL-RTTs.pdf')

