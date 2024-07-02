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
n1_df['time_diff'] =  n1_df.iloc[1:, pos] - n1_df.iat[0, pos]
# print(n1_df['time_ns'].head(15))
print(n1_df.head(15))

