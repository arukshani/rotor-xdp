import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd


### File parsing
path = "/tmp/opera-rtt.txt"

lastColumn = [ ]

text_file = open(path, "r")
lines = []

for index, text in enumerate(text_file):
    if 10000 <= index <= 70000:
        lines.append(text)

for line in lines:
    parts = line.split(' ')
    rtt_val_ms = parts[12].split('=')
    print(rtt_val_ms[1])

### End parsing