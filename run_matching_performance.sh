#!/bin/bash
source /etc/eic-env.sh

root -q -b -x -l 'matching_performance.cxx("'$1'")'
