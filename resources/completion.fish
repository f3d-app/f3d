set -l compl_cmds (f3d --help 2>&1 | sed '1,/Examples/!d' | grep "[, ] [-]-" | sed 's/-\(.\),/-s \1/g' | sed 's/=.*>//g' | sed 's/--\([^ ]*\) *\(.*\)/-l \1 -d \'\2\'/g' | sed 's/^ */complete -c f3d /')

for current_cmd in $compl_cmds; eval $current_cmd; end
