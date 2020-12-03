#!/bin/bash

while true
do
	for ((i=0; $i<20; i++))
	do
		cat /proc/modlist
		sleep 0.5
		echo remove $i > /proc/modlist
	done
done 
