#!/bin/bash

#if lsmod | grep modlist > /dev/null ; then
#	rmmod modlist	

#insmod modlist.ko

for i in {1..10};
do 
	echo add $i > /proc/modlist;
done

cat /proc/modlist;
echo remove 2 > /proc/modlist;
echo remove 8 > /proc/modlist;
cat /proc/modlist;
echo cleanup > /proc/modlist;
cat /proc/modlist;
echo add 1 > /proc/modlist;
echo add 29 > /proc/modlist;
cat /proc/modlist;
