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



xpoints = np.array([1, 2, 3, 4 , 5 ])
ypoints = np.array([12.2, 23.9, 18.42, 22.32, 20.85])

plt.plot(xpoints, ypoints)
# plt.show()
plt.savefig('ns_1q.png')