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
DATAPATH_INTERFACE="enp65s0f0np0"
NODE="node-1"

###################################################

def copy_worker_info():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            remoteCmd = 'scp -o StrictHostKeyChecking=no /tmp/all_worker_info.csv {}:/tmp'.format(worker['host'])
            proc = subprocess.run(remoteCmd, shell=True)

def write_veth_info():
     with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./get_veth_info.sh {}'.format(worker['username'], worker['host'], worker['host'])
                proc = subprocess.run(remoteCmd, shell=True)

def get_worker_mac():
    cmd = "rm -rf /tmp/all_worker_info.csv"
    subprocess.run(cmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./get_mac.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
            stdout = subprocess.run(remoteCmd, shell=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
            hex_val="0x{}".format(binascii.hexlify(socket.inet_aton(worker['ip_lan'])).decode('ascii'))
            stdout = stdout + ",{},{},{}".format(worker['username'], worker['host'], hex_val)
            with open("/tmp/all_worker_info.csv", "a") as myfile:
                myfile.write(stdout + "\n")

def change_iface_properties():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./change_iface_properties.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
                proc = subprocess.run(remoteCmd, shell=True)

def create_containers():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./create_containers.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
                proc = subprocess.run(remoteCmd, shell=True)

def setup_workers():
    with open('/tmp/workers.pkl','rb') as f:  
        workers = pickle.load(f)
        for worker in workers:
            if (worker['host'] == NODE):
                remoteCmd = 'ssh -o StrictHostKeyChecking=no {}@{} "bash -s" < ./setup_worker.sh {}'.format(worker['username'], worker['host'], worker['ip_lan'])
                proc = subprocess.run(remoteCmd, shell=True)

def main():
    # check if identity file exists & works
    if not os.path.isfile(IDENTITY_FILE):
        print('Could not find identify file: {}. Please add it to this machine to run cloudlab setup'.format(IDENTITY_FILE))
        exit(1)
    
if __name__ == '__main__':
    main()
    setup_workers()
    create_containers()
    change_iface_properties()
    get_worker_mac() ## first remove the olf /tmp/all_worker_info.csv
    write_veth_info()
    copy_worker_info()
    