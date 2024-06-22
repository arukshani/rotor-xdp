import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd

def convert_to_us(value_something):
    return value_something * 1000

def read_file(path):
    rtt_df = pd.read_csv(path, header=0, names=["rtt_ms"])
    rtt_df['rtt_us'] = rtt_df.rtt_ms.apply(convert_to_us)
    return rtt_df

direct_100us = read_file("/tmp/slot-selection/direct-slot-100us-ms.csv")
direct_200us = read_file("/tmp/slot-selection/direct-slot-200us-ms.csv")
direct_1ms = read_file("/tmp/slot-selection/direct-slot-1ms-ms.csv")

opera_100us = read_file("/tmp/slot-selection/opera-slot-100us-ms.csv")
opera_200us = read_file("/tmp/slot-selection/opera-slot-200us-ms.csv")
opera_1ms = read_file("/tmp/slot-selection/opera-slot-1ms-ms.csv")

sns.kdeplot(data = direct_100us['rtt_us'], cumulative = True, label = "Direct-100us-slot")
sns.kdeplot(data = direct_200us['rtt_us'], cumulative = True, label = "Direct-200us-slot")
sns.kdeplot(data = direct_1ms['rtt_us'], cumulative = True, label = "Direct-1ms-slot")
sns.kdeplot(data = opera_100us['rtt_us'], cumulative = True, label = "Opera-100us-slot")
sns.kdeplot(data = opera_200us['rtt_us'], cumulative = True, label = "Opera-200us-slot")
sns.kdeplot(data = opera_1ms['rtt_us'], cumulative = True, label = "Opera-1ms-slot")
plt.legend()
plt.xlabel('RTT (us)')
plt.ylabel('CDF')

# plt.xscale('log')

plt.savefig('all-slot-rtt.png')