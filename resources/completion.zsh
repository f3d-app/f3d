#compdef f3d

local longopts
local shortopts
local arguments
local f3dhelp

f3dhelp=$(f3d --help 2>&1)

shortopts=$(echo $f3dhelp | grep "\-.," | sed "s/^ *\(-.\), *--[^ ]* *\(.*\)$/\1[\2]/")
longopts=$(echo $f3dhelp | grep "\-\-" | sed 's/=.*>//g' | sed 's/-.,//g' | sed "s/^ *\([^ ]*\) *\(.*\)$/\1[\2]/g")

arguments=("${(f)shortopts}")
arguments+=("${(f)longopts}")

_arguments "$arguments[@]"
