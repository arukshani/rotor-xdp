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

path = "throughput/direct-2/"
trace_file = "2-direct-cwnd-node-1.csv"
plotname = "throughput/direct-2/plots/direct-2-cwnd.png"

trace_df = pd.read_csv(path+trace_file ,sep=',')
# print(trace_df.head(5))

# plt.plot(trace_df['time'], trace_df['srtt'], label = "srtt")
plt.plot(trace_df['time'], trace_df['snd_cwnd'], label = "snd_cwnd")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=11)
plt.xticks(fontsize=11)
plt.yticks(fontsize=11)
plt.xlabel('time', fontsize=11)
plt.ylabel('cwnd (packets)', fontsize=11)
# plt.ylabel('srtt (us)', fontsize=11)
plt.savefig(plotname)
# plt.savefig('P-ALL-RTTs.pdf')

 