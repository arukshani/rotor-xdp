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
import os
import argparse
import subprocess
import pickle
import logging
import pandas as pd
import time

def convert_tdumps():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            cmd = "./convert_tdumps.sh -f br-tdump-{}".format(worker['host'])
            subprocess.run(cmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

def main():
    convert_tdumps()

if __name__ == '__main__':
    main()