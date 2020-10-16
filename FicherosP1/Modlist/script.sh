#!/bin/bash
#if lsmod | grep modlist > /dev/null ; then
#	rmmod modlist	
#insmod modlist.ko
echo add 10 > /proc/modlist;
cat /proc/modlist;
echo add 4 > /proc/modlist;
echo add 4 > /proc/modlist;
cat /proc/modlist;
echo add 1 > /proc/modlist;
echo add 29 > /proc/modlist;
cat /proc/modlist;
echo remove 4 > /proc/modlist;
cat /proc/modlist;
echo cleanup > /proc/modlist;
cat /proc/modlist;
