import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd

#=========================TEST SAMPLE=====================
# X1 = np.arange(100)
# X2 = (X1 ** 2) / 100
# sns.kdeplot(data = X1, cumulative = True, label = "X1")
# sns.kdeplot(data = X2, cumulative = True, label = "X2")
# plt.legend()
# # plt.show()
# plt.savefig('foo.png')
# # plt.savefig('foo.pdf')
#=========================END TEST SAMPLE=================

path = "/tmp/2024-03-25_16-52-58/"

afxdp_data = pd.read_csv(path+"afxdp_fwd_delay.csv" ,sep=',', header=0,
        names=["node_ip_1", "slot_1", "topo_arr_1", "next_node_1", "time_ns_1", "time_part_sec_1", "time_part_nsec_1", "node_name"
        "node_ip_2", "slot_2", "topo_arr_2", "next_node_2", "time_ns_2", "time_part_sec_2", "time_part_nsec_2","node_name", "delay_ns"
        ])

leed_data = pd.read_csv(path+"leed-nic-forward-delay.csv" ,sep=',', header=0,
        names=["seq_id", "send_sec", "send_nsec", "recv_sec", "recv_nsec", "fwd_delay_nsec"]
        )
# node1_data['node_name'] = "node-1"

# print(leed_data.head(10))
sns.kdeplot(data = afxdp_data['delay_ns'], cumulative = True, label = "afxdp-to-afxdp")
sns.kdeplot(data = leed_data['fwd_delay_nsec'], cumulative = True, label = "fpga-to-fpga")
plt.legend()
plt.xlabel('Delay (ns)')
plt.ylabel('CDF')

# plt.xscale('log')

plt.savefig('raw_nic-to-nic.png')
