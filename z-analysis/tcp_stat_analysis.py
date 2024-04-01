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

path = "/tmp/2024-03-31_12-56-23/"
path_to_cvs_n1=path+"tcp_stats"

def clean_up_tcp_stats():
    print("clean_up_tcp_stats")

def main():
    clean_up_tcp_stats()

if __name__ == '__main__':
    main()