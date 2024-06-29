import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd

# path = "data/default-rack/"
# path = "data/default-sack/"
path_1 = "data/default-rack/"
path_2 = "data/rack-1srtt/"
path_3 = "data/rack-2srtt/"

def parse_log(file_name):
    text_file = open(file_name, "r")
    lines = []
    retransmission_count ={}
    for index, text in enumerate(text_file):
        if 4 <= index <= 65:
            lines.append(text)
    for i in range(0,60):
        # print(lines[i])
        parts = lines[i].split() 
        # print(parts[8]) 
        retransmission_count[i] = int(parts[8])
    return retransmission_count
        

# retrans_1 = parse_log(path+"opera_tcp_test1.txt")
# df_1 = pd.DataFrame(retrans_1.items(), columns=['time', 'retrans_count'])

# retrans_2 = parse_log(path+"opera_tcp_test2.txt")
# df_2 = pd.DataFrame(retrans_2.items(), columns=['time', 'retrans_count'])

# retrans_3 = parse_log(path+"opera_tcp_test3.txt")
# df_3 = pd.DataFrame(retrans_3.items(), columns=['time', 'retrans_count'])

# retrans_4 = parse_log(path+"opera_tcp_test4.txt")
# df_4 = pd.DataFrame(retrans_4.items(), columns=['time', 'retrans_count'])

# retrans_5 = parse_log(path+"opera_tcp_test5.txt")
# df_5 = pd.DataFrame(retrans_5.items(), columns=['time', 'retrans_count'])

retrans_1 = parse_log(path_1+"opera_tcp_test3.txt")
df_1 = pd.DataFrame(retrans_1.items(), columns=['time', 'retrans_count'])

retrans_2 = parse_log(path_2+"1_iperf_1srtt.txt")
df_2 = pd.DataFrame(retrans_2.items(), columns=['time', 'retrans_count'])

retrans_3 = parse_log(path_3+"2_iperf_2srtt.txt")
df_3 = pd.DataFrame(retrans_3.items(), columns=['time', 'retrans_count'])


ax = df_1.plot(x="time", y=["retrans_count"],kind="line")
df_2.plot(ax=ax, x="time", y=["retrans_count"],kind="line")
df_3.plot(ax=ax, x="time", y=["retrans_count"],kind="line")
# df_4.plot(ax=ax, x="time", y=["retrans_count"],kind="line")
# df_5.plot(ax=ax, x="time", y=["retrans_count"],kind="line")

# plt.legend(["Default-RACK", "Static 1-SRTT", "Trial 3", "Trial 4", "Trial 5"], fontsize=14)
plt.legend(["Default-RACK", "Static 1-SRTT", "Static-2-SRTT"], fontsize=14)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('Time(s)', fontsize=14)
plt.ylabel('Number of Retransmissions', fontsize=14)
# plt.savefig('P-Retrans-Default-RACK.png')
# plt.savefig('P-Retrans-Default-RACK.pdf')
# plt.savefig('P-Retrans-Default-SACK.png')
# plt.savefig('P-Retrans-Default-SACK.pdf')
# plt.savefig('P-Retrans-Custom-RACK.png')
plt.savefig('P-Retrans-Custom-RACK.pdf')

