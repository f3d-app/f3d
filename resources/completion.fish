# Convert /install/path/share/fish/vendor_completions.d/f3d.fish -> /install/path/share/f3d/cli-options.json
# The installation directory may vary, so find the json file relative to this script.
set -l script_path (status filename)
set -l share_path (path dirname (path dirname (path dirname $script_path)))
set -l json_file "$share_path/f3d/cli-options.json"

# Try to parse JSON file if it exists and python3 is available
if test -f "$json_file"; and command -q python3
    # Extract completion commands from JSON using Python
    set -l compl_cmds (python3 -c "
import json
with open('$json_file') as f:
    data = json.load(f)
for group in data.get('groups', []):
    for opt in group.get('options', []):
        long_name = opt.get('longName', '')
        short_name = opt.get('shortName', '')
        help_text = opt.get('helpText', '').replace('\"', '\\\\\"')
        cmd = 'complete -c f3d'
        if short_name:
            cmd += f' -s {short_name}'
        if long_name:
            cmd += f' -l {long_name}'
        if help_text:
            cmd += f' -d \"{help_text}\"'
        print(cmd)
" 2>/dev/null)
    
    # Execute completion commands if parsing succeeded
    if test -n "$compl_cmds"
        for current_cmd in $compl_cmds
            eval $current_cmd
        end
        exit 0
    end
end

# Fallback to parsing --help if JSON parsing failed
set -l compl_cmds (f3d --help 2>&1 | sed '1,/Examples/!d' | grep "[, ] [-]-" | sed 's/-\(.\),/-s \1/g' | sed 's/=.*>//g' | sed 's/--\([^ ]*\) *\(.*\)/-l \1 -d \'\2\'/g' | sed 's/^ */complete -c f3d /')

for current_cmd in $compl_cmds
    eval $current_cmd
end
