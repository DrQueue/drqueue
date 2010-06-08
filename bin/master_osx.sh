#!/bin/bash

export DRQUEUE_ROOT=/usr/local/drqueue
export DRQUEUE_MASTER=127.0.0.1

# start master process
$DRQUEUE_ROOT/bin/master.Darwin.fat

