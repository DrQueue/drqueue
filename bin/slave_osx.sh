#!/bin/bash

[ -z $DRQUEUE_ROOT ]   && export DRQUEUE_ROOT=/usr/local/drqueue
[ -z $DRQUEUE_MASTER ] && export DRQUEUE_MASTER=127.0.0.1

# start slave process
$DRQUEUE_ROOT/bin/slave.Darwin.fat

