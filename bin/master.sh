#!/bin/sh
#
# $Id: master.sh,v 1.1 2002/02/21 11:45:50 jorge Exp $
#

if [ -f /proc/cpuinfo ]; then
	echo "Linux"
	./master.linux
else
	echo "Irix"
	./master.irix
fi
