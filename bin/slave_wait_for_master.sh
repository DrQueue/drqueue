#!/bin/bash

# 2009, andreas@drqueue.org
# wait until master is reachable
# this might be useful as a startup script

nc -v -w 2 -z $DRQUEUE_MASTER 1234
EXSTAT=$?

while [ $EXSTAT -eq 1 ]
      do
        echo "waiting 5 seconds for master to come up ..."
	sleep 5
	nc -v -w 2 -z $DRQUEUE_MASTER 1234
	EXSTAT=$?
      done

${DRQUEUE_ROOT}/bin/slave.Darwin.i386 -o

exit 0



