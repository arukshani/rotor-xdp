import csv
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import re
import plotly.graph_objs as go
import plotly
import plotly.express as px
import matplotlib.pyplot as plt
import re
import numpy as np
import seaborn as sns

dataframe_collection ={}
# topo_hop_dict = {}
# some_dict[a] = b
# some_dict[3] = 4

def load_data():
    for num in range(1, 33):
        node_path = "/opt/rotor-xdp/opera-v2/config/node-{}.csv".format(num)
        df = pd.read_csv(node_path ,sep=',', header=None)
        dataframe_collection[num] = df

def get_next_node(src,dst,topo,num_hops, topo_hop_dict):
    num_hops = num_hops+1
    # print(" n{} ".format(src), end = " ")
    src_df = dataframe_collection[src]
    dst_row = src_df.iloc[dst-1]
    if(dst_row[topo-1] == dst):
        num_hops = num_hops+1
        # print("n{}, hops-{}".format(dst, num_hops))
        topo_hop_dict[topo] = num_hops
        # print(num_hops)
    else:
        return get_next_node(dst_row[topo-1],dst,topo,num_hops, topo_hop_dict)
       

def get_path_per_topo(src,dst, topo_hop_dict):
    src_df = dataframe_collection[src]
    dst_row = src_df.iloc[dst-1]
    for num in range(0, 32):
        num_hops = 0
        if(dst_row[num] == dst):
            num_hops = num_hops+1
            # print("topo-{}, n{}->n{}, hops-{}".format(num+1, src, dst,num_hops))
            topo_hop_dict[num+1] = num_hops
            # print(num_hops)
        else:
            # print("Find next nodes for topo-{}".format(num+1))
            get_next_node(dst_row[num], dst, num+1, num_hops, topo_hop_dict)
            

def main(args):
    load_data()

    fwd_topo_hop_dict = {}
    ret_topo_hop_dict = {}
    # total_hop_dict = {}

    get_path_per_topo(1,2, fwd_topo_hop_dict)
    # print(fwd_topo_hop_dict)

    get_path_per_topo(2,1, ret_topo_hop_dict)
    # print(ret_topo_hop_dict)

    tot_hops = []
    for x in range(1, 33):
        # print("We're on time %d" % (x))
        # total_hop_dict[x] = fwd_topo_hop_dict[x] + ret_topo_hop_dict[x]
        tot=fwd_topo_hop_dict[x] + ret_topo_hop_dict[x]
        tot_hops.append(tot)
    
    print(tot_hops)
    dataframe=pd.DataFrame(tot_hops, columns=['tot_hops']) 
    print(dataframe['tot_hops'])
    # sns.kdeplot(data = dataframe['tot_hops'], cumulative = True, label = "n1-n2")

    fig, ax = plt.subplots()
    sns.ecdfplot(data=dataframe, x="tot_hops", ax=ax, label = "n1-n2")

    plt.legend(fontsize=14)
    plt.xticks(fontsize=11)
    plt.yticks(fontsize=11)
    plt.xlabel('ideal hop count', fontsize=16)
    plt.ylabel('CDF', fontsize=16)
    plt.savefig('ideal-hops.png')
        
        
        
def parse_args():
    parser = argparse.ArgumentParser(description='Analayze Data')
    parser.add_argument('--analyze', '-a', action='store_true')
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = parse_args()
    main(args)