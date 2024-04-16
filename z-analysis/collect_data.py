import os
import subprocess
import ipaddress
import pandas as pd
import pickle
import json 
# import constant
import binascii
import socket
import errno
from datetime import datetime

USER = os.environ['USER']
IDENTITY_FILE = '/users/{}/.ssh/{}_cloudlab.pem'.format(USER, USER)

def collect_tcp_stat_logs():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        mydir = os.path.join(
            "/tmp/", 
            datetime.now().strftime('%Y-%m-%d_%H-%M-%S'))
        # print(mydir)
        try:
            os.makedirs(mydir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise  # This was not a "directory exist" error..
        for worker in workers:
            remoteCmd = 'scp -o StrictHostKeyChecking=no {}:/tmp/tcp_stats {}'.format(worker['host'], mydir)
            proc = subprocess.run(remoteCmd, shell=True)
            new_filename = "tcp-stats-{}".format(worker['host'])
            cmd = "mv {}/tcp-stats {}/{}".format(mydir, mydir, new_filename)
            subprocess.run(cmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

def gather_data():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        mydir = os.path.join(
            "/tmp/XDP/", 
            datetime.now().strftime('%Y-%m-%d_%H-%M-%S'))
        # print(mydir)
        try:
            os.makedirs(mydir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise  # This was not a "directory exist" error..
        for worker in workers:
            remoteCmd = 'scp -o StrictHostKeyChecking=no {}:/tmp/opera_emu_data.csv {}'.format(worker['host'], mydir)
            proc = subprocess.run(remoteCmd, shell=True)
            new_filename = "afxdp-data-{}.csv".format(worker['host'])
            cmd = "mv {}/opera_emu_data.csv {}/{}".format(mydir, mydir, new_filename)
            subprocess.run(cmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

def gather_tdumps():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        mydir = os.path.join(
            "/tmp/TDUMPS/", 
            datetime.now().strftime('%Y-%m-%d_%H-%M-%S'))
        # print(mydir)
        try:
            os.makedirs(mydir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise  # This was not a "directory exist" error..
        for worker in workers:
            remoteCmd = 'scp -o StrictHostKeyChecking=no {}:/tmp/host-br_time.pcap {}'.format(worker['host'], mydir)
            proc = subprocess.run(remoteCmd, shell=True)
            new_filename = "br-tdump-{}.pcap".format(worker['host'])
            cmd = "mv {}/host-br_time.pcap {}/{}".format(mydir, mydir, new_filename)
            subprocess.run(cmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()

def main():
    # check if identity file exists & works
    if not os.path.isfile(IDENTITY_FILE):
        print('Could not find identify file: {}. Please add it to this machine to run cloudlab setup'.format(IDENTITY_FILE))
        exit(1)
    
if __name__ == '__main__':
    main()
    gather_data()
    # collect_tcp_stat_logs()
    # gather_tdumps()