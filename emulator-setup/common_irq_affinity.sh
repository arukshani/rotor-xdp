#!/bin/bash
#
# Copyright (c) 2017 Mellanox Technologies. All rights reserved.
#
# This Software is licensed under one of the following licenses:
#
# 1) under the terms of the "Common Public License 1.0" a copy of which is
#    available from the Open Source Initiative, see
#    https://urldefense.com/v3/__http://www.opensource.org/licenses/cpl.php__;!!Mih3wA!E0ciAR3JeSmn8fn4eWEalf3A3Kz-4J8-AT07_gky_31WhJLfX3LUtUtubgqq9fzrc2pnLypA-u_r9rmWrBI$ .
#
# 2) under the terms of the "The BSD License" a copy of which is
#    available from the Open Source Initiative, see
#    https://urldefense.com/v3/__http://www.opensource.org/licenses/bsd-license.php__;!!Mih3wA!E0ciAR3JeSmn8fn4eWEalf3A3Kz-4J8-AT07_gky_31WhJLfX3LUtUtubgqq9fzrc2pnLypA-u_rTa1U-9s$ .
#
# 3) under the terms of the "GNU General Public License (GPL) Version 2" a
#    copy of which is available from the Open Source Initiative, see
#    https://urldefense.com/v3/__http://www.opensource.org/licenses/gpl-license.php__;!!Mih3wA!E0ciAR3JeSmn8fn4eWEalf3A3Kz-4J8-AT07_gky_31WhJLfX3LUtUtubgqq9fzrc2pnLypA-u_r0cfUf9M$ .
#
# Licensee has the right to choose one of the above licenses.
#
# Redistributions of source code must retain the above copyright
# notice and one of the license notices.
#
# Redistributions in binary form must reproduce both the above copyright
# notice, one of the license notices in the documentation
# and/or other materials provided with the distribution.
#

function add_comma_every_eight
{
        echo " $1 " | sed -r ':L;s=\b([0-9]+)([0-9]{8})\b=\1,\2=g;t L'
}

function int2hex
{
	CHUNKS=$(( $1/64 ))
	COREID=$1
	HEX=""
	for (( CHUNK=0; CHUNK<${CHUNKS} ; CHUNK++ ))
	do
		HEX=$HEX"0000000000000000"
		COREID=$((COREID-64))
	done
        printf "%x$HEX" $(echo $((2**$COREID)) )
}


function core_to_affinity
{
	echo $( add_comma_every_eight $( int2hex $1) )
}

function get_irq_list
{
	interface=$1
	pci_dev=$(ethtool -i $interface | grep "bus-info:" | cut -d ' ' -f 2)
	infiniband_device_irqs_path="/sys/class/infiniband/$interface/device/msi_irqs"
	net_device_irqs_path="/sys/class/net/$interface/device/msi_irqs"
	interface_in_proc_interrupts=$(grep -P "$interface[^0-9,a-z,A-Z]" /proc/interrupts | cut -d":" -f1)
	pci_in_proc_interrupts=$(grep "$pci_dev" /proc/interrupts | grep -v "async" | cut -d":" -f1)

	if [ -d "$infiniband_device_irqs_path" ]; then
		irq_list=$(/bin/ls -1 "$infiniband_device_irqs_path" | tail -n +2)
	elif [ "$interface_in_proc_interrupts" != "" ]; then
		irq_list=$interface_in_proc_interrupts
	elif [ "$pci_in_proc_interrupts" != "" ]; then
		irq_list=$pci_in_proc_interrupts
	elif [ -d "$net_device_irqs_path" ]; then
		irq_list=$(/bin/ls -1 "$net_device_irqs_path" | tail -n +2)
	else
		echo "Error - interface or device \"$interface\" does not exist" 1>&2
		exit 1
	fi
	sorted_irq_list=$(echo "$irq_list" | sort -g)
	echo "$sorted_irq_list"
}

function show_irq_affinity
{
	irq_num=$1
	show_cpu_number=$2
	smp_affinity_path="/proc/irq/$irq_num/smp_affinity"
	cpu_number_path="/proc/irq/$irq_num/smp_affinity_list"
	if [ -f $smp_affinity_path ]; then
		if [ -f $cpu_number_path ] && [ "$show_cpu_number" == "show_cpu_number" ]; then
			echo -n "$irq_num (cpu #`cat $cpu_number_path`): "
		else
			echo -n "$irq_num: "
		fi
		cat $smp_affinity_path
	fi
}

function show_irq_affinity_hints
{
	irq_num=$1
	affinity_hint_path="/proc/irq/$irq_num/affinity_hint"
        if [ -f $affinity_hint_path ]; then
                echo -n "$irq_num: "
                cat $affinity_hint_path
        fi
}

function set_irq_affinity
{
	irq_num=$1
	affinity_mask=$2
	smp_affinity_path="/proc/irq/$irq_num/smp_affinity"
        if [ -f $smp_affinity_path ]; then
                echo $affinity_mask > $smp_affinity_path
        fi
}

function is_affinity_hint_set
{
	irq_num=$1
	hint_not_set=0
	affinity_hint_path="/proc/irq/$irq_num/affinity_hint"
	if [ -f $affinity_hint_path ]; then
		TOTAL_CHAR=$( wc -c < $affinity_hint_path  )
		NUM_OF_COMMAS=$( grep -o "," $affinity_hint_path | wc -l )
		NUM_OF_ZERO=$( grep -o "0" $affinity_hint_path | wc -l )
		NUM_OF_F=$( grep -i -o "f" $affinity_hint_path | wc -l )
		if [[ $((TOTAL_CHAR-1-NUM_OF_COMMAS)) -eq $NUM_OF_ZERO || $((TOTAL_CHAR-1-NUM_OF_COMMAS)) -eq $NUM_OF_F ]]; then
			hint_not_set=1
		fi
	else
		hint_not_set=1
	fi
	return $hint_not_set
}