#!/bin/sh
#
# $Id: slave.sh,v 1.1 2002/02/21 11:45:50 jorge Exp $
#

if [ -f /proc/cpuinfo ]; then
	echo "Linux"
	./slave.linux
else
	echo "Irix"
	./slave.irix
fi
