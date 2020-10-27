#!/bin/bash
#if lsmod | grep modlist > /dev/null ; then
#	rmmod modlist	
#insmod modlist.ko
echo add aaaaaaaaaa > /proc/modlist;
cat /proc/modlist;
echo add bbbb > /proc/modlist;
echo add bbbb > /proc/modlist;
cat /proc/modlist;
echo add ccc > /proc/modlist;
echo add ccc > /proc/modlist;
cat /proc/modlist;
echo remove ccc > /proc/modlist;
cat /proc/modlist;
echo cleanup > /proc/modlist;
cat /proc/modlist;
