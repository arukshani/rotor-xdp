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

def convert_to_float(value_something):
    return float(value_something)

def extract_value(value_something):
    return float(value_something.split('=', 1)[1])

def extract_reo(reo_wnd):
    return float(reo_wnd.split(':', 1)[1])

def clean_time(log_time):
    cleaned_val = re.search(r'\[(.+)\]', log_time) 
    return cleaned_val.group(1)

path = "/opt/rotor-xdp/z-analysis/"
path_to_cvs=path+"lost_reo_wnd.csv"
df = pd.DataFrame()

skb_lost_data = pd.read_csv(path_to_cvs ,sep=',', header=0,
        names=["time", "reo_wnd", "remaining", "rack_rtt_us"])

# node2_results.index = (node2_results.epoch_msec - node2_results.epoch_msec[0]) 
df['log_time'] = skb_lost_data.time.apply(clean_time)
df['log_time_float'] = df.log_time.apply(convert_to_float)
df['time_elapsed_s'] = df.log_time_float - df.log_time_float[0]
df['remaining'] = skb_lost_data.remaining.apply(extract_value)
df['remaining_abs'] = abs(df['remaining'])
df['reo_wnd'] = skb_lost_data.reo_wnd.apply(extract_reo)
df['rack_rtt_us'] = skb_lost_data.rack_rtt_us.apply(extract_value)
df['full_wait_time'] = df['rack_rtt_us'] + df['reo_wnd']
df['skb_timeout'] = df['rack_rtt_us'] + df['reo_wnd'] + abs(df['remaining'])
print(df.head(5))

# df.plot(x="time_elapsed_s", y=["reo_wnd", "rack_rtt_us", "full_wait_time", "skb_timeout"],
#         kind="line")

# plt.legend()
# plt.xlabel('time')
# plt.ylabel('us')
# plt.savefig('reo.jpg')

# fig = px.line(df, x='time_elapsed_s', y='reo_wnd')
# fig.write_html("skb_lost_plot.html")
fig = go.Figure()
fig.add_traces(go.Scatter(x=df['time_elapsed_s'], y = df['reo_wnd'], mode = 'lines', name="reo_wnd"))
fig.add_traces(go.Scatter(x=df['time_elapsed_s'], y = df['rack_rtt_us'], mode = 'lines', name="rack_rtt_us"))
fig.add_traces(go.Scatter(x=df['time_elapsed_s'], y = df['skb_timeout'], mode = 'lines', name="skb_timeout"))
fig.add_traces(go.Scatter(x=df['time_elapsed_s'], y = df['full_wait_time'], mode = 'lines', name="full_wait_time"))
fig.add_traces(go.Scatter(x=df['time_elapsed_s'], y = df['remaining_abs'], mode = 'lines', name="remaining_abs"))
# fig.write_image("reo.jpg")
fig.write_html("skb_lost_plot.html")

