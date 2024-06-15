import os
import subprocess
import ipaddress
import pandas as pd
import pickle
import json 
# import constant
import binascii
import socket
import argparse
import logging

USER = os.environ['USER']
IDENTITY_FILE = '/users/{}/.ssh/{}_cloudlab.pem'.format(USER, USER)
# NODE="node-1"

def build_kernel(node_name):
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == node_name):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./build_kernel.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def prepare_kernel(node_name):
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == node_name):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./prepare_kernel.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def main(args):
    # check if identity file exists & works
    if not os.path.isfile(IDENTITY_FILE):
        print('Could not find identify file: {}. Please add it to this machine to run cloudlab setup'.format(IDENTITY_FILE))
        exit(1)

    if(args.nodename):
        node_name = args.nodename
        # print(NODE)

    if(args.prepare):
        prepare_kernel(node_name)
        # print("prepare")
    
    if(args.build):
        build_kernel(node_name)
        # print("build")
    

def parse_args():
    parser = argparse.ArgumentParser(description='Prepare and build kernel')
    parser.add_argument('nodename')
    parser.add_argument('--prepare', '-p', action='store_true')
    parser.add_argument('--build', '-b', action='store_true')
    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = parse_args()
    logging.info('Arguments: {}'.format(args))
    main(args)