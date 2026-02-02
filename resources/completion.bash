_f3d()
{
    local cur prev opts
    _init_completion || return

    # Convert /install/path/share/bash_completion/completions/f3d -> /install/path/share/f3d/cli-options.json
    # The installation directory may vary, so find the json file relative to this script.
    local script_path
    script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -L)
    local share_path="${script_dir%/*/*}"
    local json_file="${share_path}/f3d/cli-options.json"
    
    # If current word starts with -, provide option completions
    if [[ $cur == -* ]]; then
        # Extract options from JSON file if it exists
        if [[ -f "$json_file" ]] && command -v python3 &> /dev/null; then
            opts=$(python3 -c "
import json
with open('$json_file') as f:
    data = json.load(f)
opts = []
for group in data.get('groups', []):
    for opt in group.get('options', []):
        if opt.get('longName'):
            opts.append('--' + opt['longName'])
        if opt.get('shortName'):
            opts.append('-' + opt['shortName'])
print(' '.join(sorted(set(opts))))
" 2>/dev/null)
            
            # If Python parsing failed, fallback to --help
            if [[ -z "$opts" ]]; then
                opts=$(_parse_help "$1" --help)
            fi
        else
            # Fallback to --help if JSON file doesn't exist or python is not installed
            opts=$(_parse_help "$1" --help)
        fi
        
        COMPREPLY=( $(compgen -W "$opts" -- "$cur") )
        return
    fi

    # For file arguments
    _filedir
} &&
complete -F _f3d f3d
