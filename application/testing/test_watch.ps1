# Test the watch feature by opening an innexistent file
# rewriting it as invalid, rewriting as lower resolution
# and checking the the file has been automatically reloaded

$f3d_cmd = $args[0]
$data_dir = $args[1]
$tmp_dir = $args[2]

$hires_data = "$data_dir/cow.vtp"
$lowres_data = "$data_dir/cowlow.vtp"
$invalid_data = "$data_dir/invalid_body.vtp"
$reloaded_data = "$tmp_dir/cow.vtp"

Remove-Item $reloaded_data

$log = "$tmp_dir/output.log"
$id = (Start-Process -FilePath $f3d_cmd -ArgumentList "--watch --verbose $reloaded_data" -RedirectStandardOutput $log -PassThru).Id

Start-Sleep -Seconds 1
Copy-Item $hires_data -Destination $reloaded_data
Start-Sleep -Seconds 1
Copy-Item $invalid_data -Destination $reloaded_data
Start-Sleep -Seconds 1
Copy-Item $lowres_data -Destination $reloaded_data
Start-Sleep -Seconds 1

$str = Select-String $log -Pattern "Number of points: 634"
if ($str -ne $null) {
  $ret = 0
} else {
  $ret = 1
}

Stop-Process -Id $id
exit $ret
