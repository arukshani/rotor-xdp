import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd

def convert_to_us(value_something):
    return value_something * 1000

path = "/tmp/direct_rtt_ms.csv"
direct_rtt = pd.read_csv(path, header=0, names=["rtt_ms"])
direct_rtt['rtt_us'] = direct_rtt.rtt_ms.apply(convert_to_us)

opera_path = "tmp/opera_rtt_ms.csv"
opera_rtt = pd.read_csv(opera_path, header=0, names=["rtt_ms"])
opera_rtt['rtt_us'] = opera_rtt.rtt_ms.apply(convert_to_us)

# print(direct_rtt.head(5))
sns.kdeplot(data = direct_rtt['rtt_us'], cumulative = True, label = "Direct-RTT")
sns.kdeplot(data = opera_rtt['rtt_us'], cumulative = True, label = "Opera-RTT")
plt.legend()
plt.xlabel('RTT (us)')
plt.ylabel('CDF')

# plt.xscale('log')

plt.savefig('all-rtt-200r.png')