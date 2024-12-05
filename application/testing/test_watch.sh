#!/bin/bash

# Test the watch feature by opening a file
# rewriting it and checking the the file has been
# automatically reloaded

set -euo pipefail
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

function cleanup()
{
  kill -SIGTERM $pid
}
trap "cleanup" EXIT

sleep 3
cp $lowres_data $reloaded_data
sleep 3

grep -q "Number of points: 634" $log
