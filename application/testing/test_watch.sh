#!/bin/bash

# Test the watch feature by opening a file
# rewriting it and checking the the file has been
# automatically reloaded

set -uo pipefail
f3d_cmd=$1
data_dir=$2
tmp_dir=$3

hires_data=$data_dir/cow.vtp
lowres_data=$data_dir/cowlow.vtp
reloaded_data=$tmp_dir/cow.vtp

cp $hires_data $reloaded_data

log=$tmp_dir/output.log
$f3d_cmd --watch --verbose $reloaded_data > $log &
pid=$!

sleep 1
cp $lowres_data $reloaded_data
sleep 1

grep -q "Number of points: 634" $log
ret=$?

kill $pid
exit $ret
