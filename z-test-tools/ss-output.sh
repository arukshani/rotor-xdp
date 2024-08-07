#!/bin/bash

DST=$1

touch /tmp/sender-ss.txt

rm -f /tmp/sender-ss.txt 

# cleanup ()
# {
# 	# get timestamp
# 	ts=$(cat sender-ss.txt |   sed -e ':a; /<->$/ { N; s/<->\n//; ba; }' | grep "ESTAB"  |  grep "unacked" |  awk '{print $1}')

# 	# get sender
# 	sender=$(cat sender-ss.txt |   sed -e ':a; /<->$/ { N; s/<->\n//; ba; }' | grep "ESTAB"  | grep "unacked" | awk '{print $6}')


# 	# retransmissions - current, total
# 	retr=$(cat sender-ss.txt |   sed -e ':a; /<->$/ { N; s/<->\n//; ba; }' | grep "ESTAB"  |  grep -oP '\bunacked:.*\brcv_space'  | awk -F '[:/ ]' '{print $4","$5}' | tr -d ' ')


# 	# get cwnd, ssthresh
# 	cwn=$(cat sender-ss.txt |   sed -e ':a; /<->$/ { N; s/<->\n//; ba; }' | grep "ESTAB"    |  grep "unacked" | grep -oP '\bcwnd:.*(\s|$)\bbytes_acked' | awk -F '[: ]' '{print $2","$4}')

# 	# get smoothed RTT (in ms)
# 	srtt=$(cat sender-ss.txt |   sed -e ':a; /<->$/ { N; s/<->\n//; ba; }' | grep "ESTAB"  |  grep "unacked" | grep -oP '\brtt:[0-9]+\.[0-9]+'  | awk -F '[: ]' '{print $2}')

# 	# concatenate into one CSV
# 	paste -d ',' <(printf %s "$ts") <(printf %s "$sender") <(printf %s "$retr") <(printf %s "$cwn") <(printf %s "$srtt") > sender-ss.csv


# 	exit 0
# }

# trap cleanup SIGINT SIGTERM

while [ 1 ]; do 
	# ss --no-header -ein dst $DST | ts '%.s' | tee -a sender-ss.txt 
	# ss --no-header -in dst $DST | ts '%.s' | tee -a sender-ss.txt
	# ss --no-header -it '( dport = :5000 )'
	ss --no-header -it '( dport = :5000 )' | ts '%.s' | tee -a /tmp/sender-ss.txt 
	# watch -n 0.00001 ss --no-header -eit '( dport = :5000 )' | ts '%.s' | while read pong; do echo "$(date):$pong"; done >> /tmp/sender-ss.txt 
done