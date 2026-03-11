#compdef f3d

local longopts
local shortopts
local arguments
local f3dhelp

f3dhelp=$(f3d --help 2>&1 | sed '1,/Examples/!d' | sed 's/\[.*\] *//')

shortopts=$(echo $f3dhelp | grep "^\s*[-]., --" | sed "s/^ *\(-.\), *--[^ ]* *\(.*\)$/\1[\2]/")
longopts=$(echo $f3dhelp | grep "[, ] [-]-" | sed 's/=.*>//g' | sed 's/-.,//g' | sed "s/^ *\([^ ]*\) *\(.*\)$/\1[\2]/g")

arguments=("${(f)shortopts}")
arguments+=("${(f)longopts}")

_arguments "$arguments[@]" "*:file:_files"
