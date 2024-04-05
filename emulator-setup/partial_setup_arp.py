import subprocess
import binascii
import socket
import pandas as pd
import logging
import argparse
import pickle

# NSes = ["blue", "red", "ns12", "ns13", "ns15", "ns16", "ns17", "ns18", "ns19", "ns20", "ns21", "ns22", "ns23", "ns24"]
# NSes = ["ns1", "ns2", "ns3", "ns4", "ns5", "ns6", "ns7", "ns8"]
NSes = ["ns1", "ns2"]
NODE="node-1"

def remove_all_arp_records():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./remove_all_arp.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
            proc = subprocess.run(remoteCmd, shell=True)

def add_arp_records():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker_in in workers:
            for worker in workers:
                if (worker_in['host'] != worker['host']):
                    filename="/tmp/NS" +  worker['host'] + ".csv"
                    arp_info = pd.read_csv(filename, header=None)
                    # for i in range(len(NSes)):
                    for index, row in arp_info.iterrows():
                        print("Logged into {}: exectute {} \n".format(worker_in['host'], worker['host']))
                        print("{} {} {} {} \n".format(row[2], row[3], row[4], row[5]))
                        remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./add_arp.sh {} {} {} {}'.format(worker_in['username'], worker_in['host'], row[2], row[3], row[4], row[5])
                        # print(remoteCmd + "\n")
                        proc = subprocess.run(remoteCmd, shell=True)
                    print("++++++++++++++")

def copy_nsinfo_to_master():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == NODE):
                filename="NS" +  worker['host'] + ".csv"
                remoteCmd = 'scp -o StrictHostKeyChecking=no {}:/tmp/{} /tmp/ '.format(worker['host'], filename)
                proc = subprocess.run(remoteCmd, shell=True)

def parse_args():
    parser = argparse.ArgumentParser(description='filename')
    parser.add_argument('filename')
    args = parser.parse_args()
    return args
    
if __name__ == '__main__':
    # args = parse_args()
    # print('Arguments: {}'.format(args.ip_of_node))
    
    copy_nsinfo_to_master()
    remove_all_arp_records() ##new
    add_arp_records()