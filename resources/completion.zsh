#compdef f3d

# Convert /install/path/share/zsh/site-functions/_f3d -> /install/path/share/f3d/cli-options.json
# The installation directory may vary, so find the json file relative to this script.
local share_path="${${(%):-%x}:a:h:h:h}"
local json_file="${share_path}/f3d/cli-options.json"
local arguments

if [[ -f "$json_file" ]] && command -v python3 &> /dev/null; then
    # Parse JSON file using Python
    local opts_data
    opts_data=$(python3 -c "
import json
with open('$json_file') as f:
    data = json.load(f)
for group in data.get('groups', []):
    for opt in group.get('options', []):
        long_name = opt.get('longName', '')
        short_name = opt.get('shortName', '')
        help_text = opt.get('helpText', '')
        if short_name:
            print(f'-{short_name}[{help_text}]')
        if long_name:
            print(f'--{long_name}[{help_text}]')
" 2>/dev/null)
    
    if [[ -n "$opts_data" ]]; then
        arguments=("${(f)opts_data}")
    fi
fi

# Fallback to parsing --help if JSON parsing failed
if [[ -z "$arguments" ]]; then
    local f3dhelp
    f3dhelp=$(f3d --help 2>&1 | sed '1,/Examples/!d' | sed 's/\[.*\] *//')
    
    local shortopts
    local longopts
    
    shortopts=$(echo $f3dhelp | grep "^\s*[-]., --" | sed "s/^ *\(-.\), *--[^ ]* *\(.*\)$/\1[\2]/")
    longopts=$(echo $f3dhelp | grep "[, ] [-]-" | sed 's/=.*>//g' | sed 's/-.,//g' | sed "s/^ *\([^ ]*\) *\(.*\)$/\1[\2]/g")
    
    arguments=("${(f)shortopts}")
    arguments+=("${(f)longopts}")
fi

_arguments "$arguments[@]" "*:file:_files"
