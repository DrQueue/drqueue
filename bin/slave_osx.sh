#!/bin/bash

export DRQUEUE_ROOT=/usr/local/drqueue
export DRQUEUE_MASTER=127.0.0.1

# start slave process
$DRQUEUE_ROOT/bin/slave.Darwin.fat

