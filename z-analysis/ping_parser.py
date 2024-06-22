import numpy as np
from matplotlib import pyplot as plt
import seaborn as sns
import argparse
import logging
import pandas as pd


### File parsing
# path = "/tmp/slot-selection/direct-slot-100us.txt"
# path = "/tmp/slot-selection/direct-slot-200us.txt"
# path = "/tmp/slot-selection/direct-slot-1ms.txt"

# path = "/tmp/slot-selection/opera-slot-100us.txt"
# path = "/tmp/slot-selection/opera-slot-200us.txt"
# path = "/tmp/slot-selection/opera-slot-1ms.txt"

lastColumn = [ ]

text_file = open(path, "r")
lines = []

for index, text in enumerate(text_file):
    if 10000 <= index <= 60000:
        lines.append(text)

for line in lines:
    parts = line.split(' ')
    rtt_val_ms = parts[12].split('=')
    print(rtt_val_ms[1])

### End parsing