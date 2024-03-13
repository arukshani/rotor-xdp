#!/bin/sh

sudo apt-get -y update
sudo apt -y install python3-pip
pip3 install pandas
pip3 install plotly
pip3 install matplotlib
sudo apt -y install htop