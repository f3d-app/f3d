# Test the watch feature by opening a file
# rewriting it and checking the the file has been
# automatically reloaded

$f3d_cmd = $args[0]
$data_dir = $args[1]
$tmp_dir = $args[2]

$hires_data = "$data_dir/cow.vtp"
$lowres_data = "$data_dir/cowlow.vtp"
$reloaded_data = "$tmp_dir/cow.vtp"

Copy-Item $hires_data -Destination $reloaded_data

$log = "$tmp_dir/output.log"
$id = (Start-Process -FilePath $f3d_cmd -ArgumentList "--watch --verbose $reloaded_data" -RedirectStandardOutput $log -PassThru).Id

Start-Sleep -Seconds 3
Copy-Item $lowres_data -Destination $reloaded_data
Start-Sleep -Seconds 3

$str = Select-String $log -Pattern "Number of points: 634"
if ($str -ne $null) {
  $ret = 0
} else {
  $ret = 1
}

Stop-Process -Id $id
exit $ret
