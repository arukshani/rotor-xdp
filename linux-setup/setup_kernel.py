import os
import subprocess
import ipaddress
import pandas as pd
import pickle
import json 
# import constant
import binascii
import socket

USER = os.environ['USER']
IDENTITY_FILE = '/users/{}/.ssh/{}_cloudlab.pem'.format(USER, USER)

def build_kernel():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == "node-1"):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./build_kernel.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def prepare_kernel():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == "node-1"):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./prepare_kernel.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def main():
    # check if identity file exists & works
    if not os.path.isfile(IDENTITY_FILE):
        print('Could not find identify file: {}. Please add it to this machine to run cloudlab setup'.format(IDENTITY_FILE))
        exit(1)
    
if __name__ == '__main__':
    main()
    # prepare_kernel()
    build_kernel()