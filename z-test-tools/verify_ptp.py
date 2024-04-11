import os
import argparse
import subprocess
import pickle
import logging
import pandas as pd
import time

CLIENT_NODE="node-1"

def kill_broadcast_server():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] != CLIENT_NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./kill_br_server.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def send_broadcast_msg():
     with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == CLIENT_NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./start_br_client.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)


def start_broadcast_server():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] != CLIENT_NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./start_br_server.sh'.format(worker['username'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def start_tdump():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] != CLIENT_NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./start_tdump.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
                proc = subprocess.run(remoteCmd, shell=True)

def main():
    start_tdump()
    start_broadcast_server()
    send_broadcast_msg()
    time.sleep(3)
    kill_broadcast_server()

if __name__ == '__main__':
    main()