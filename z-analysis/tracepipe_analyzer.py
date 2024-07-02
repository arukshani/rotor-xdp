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

path = "data/cwnd_logs/"

trace_file = "direct-no-ratelimit.csv"

trace_df = pd.read_csv(path+trace_file ,sep=',')
print(trace_df.head(5))

plt.plot(trace_df['time'], trace_df['srtt'], label = "srtt")

# fig, ax = plt.subplots()
# sns.ecdfplot(data=trace_df, x="snd_cwnd", ax=ax, label = "cwnd")

plt.legend(fontsize=14)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
# plt.xlabel('time', fontsize=14)
# plt.ylabel('srtt', fontsize=14)
plt.savefig('srtt_no-ratelimit.png')
# plt.savefig('P-ALL-RTTs.pdf')

 