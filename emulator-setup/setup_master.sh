#!/bin/sh

sudo apt-get -y update
sudo apt -y install python3-pip
pip3 install pandas
pip3 install plotly
pip3 install matplotlib
pip3 install seaborn
pip3 install scipy
sudo apt -y install htop
pip install -U kaleido
# sudo apt -y install tshark