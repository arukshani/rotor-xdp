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


names = ['1_1', '2_2', '3_3', '4_4', '5_5', '6_6', '7_7']
values_1 = [20.2, 37.2, 52.9, 65, 77.6, 85.3, 90.2]
values_2 = [9.41, 18.46, 25.03, 32.73, 39.74, 42.38, 48.58]
plt.plot(names, values_1, label = "Cloudlab-MLX-DRV-MODE")
plt.plot(names, values_2, label = "LEED-FPGA-NIC-SKB-MODE")

plt.legend()
plt.xlabel('NamespaceCount_HWQueueCount', fontsize=16)
plt.ylabel('Throughput (Gbps)', fontsize=16)
plt.savefig('throughput_1_1.pdf')

# names = ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11']
# values_1 = [18, 33, 50, 63, 78, 87, 87, 90, 85, 92, 90]
# values_2 = [11, 22, 30, 38, 37, 53, 65, 65, 63, 70, 74]
# plt.plot(names, values_1, label = "FPGA-SendRate")
# plt.plot(names, values_2, label = "FPGA-ReceiveRate")

# plt.legend()
# plt.xlabel('HWQueueCount', fontsize=16)
# plt.ylabel('Throughput (Gbps)', fontsize=16)
# plt.savefig('packet_gen.pdf')

