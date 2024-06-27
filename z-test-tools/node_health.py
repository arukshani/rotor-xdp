import os
import argparse
import subprocess
import pickle
import logging
import pandas as pd

def emu_process_check():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            print("===================READ start log==={}=======================".format(worker['host']))
            remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./process_check.sh'.format(worker['username'],worker['host'])
            proc = subprocess.run(remoteCmd, shell=True)

def delete_logs():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            print("===================DELETE logs==={}=======================".format(worker['host']))
            remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./delete_logs.sh'.format(worker['username'],worker['host'])
            proc = subprocess.run(remoteCmd, shell=True)

def read_start_log():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            print("===================READ start log==={}=======================".format(worker['host']))
            remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./read_start_log.sh'.format(worker['username'],worker['host'])
            proc = subprocess.run(remoteCmd, shell=True)

def main(args):
    # print(args)
    if(args.read):
        read_start_log()
    if(args.delete):
        delete_logs()
    if(args.check):
        emu_process_check()

def parse_args():
    parser = argparse.ArgumentParser(description='Start and stop PTP on worker nodes')
    parser.add_argument('--read', '-r', action='store_true')
    parser.add_argument('--delete', '-d', action='store_true')
    parser.add_argument('--check', '-c', action='store_true')
    args = parser.parse_args()
    return args
    
if __name__ == '__main__':
    args = parse_args()
    logging.info('Arguments: {}'.format(args))
    main(args)