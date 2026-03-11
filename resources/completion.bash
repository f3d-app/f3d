
_f3d()
{
    local cur
    _init_completion || return

    if [[ $cur == -* ]]; then
        COMPREPLY=( $(compgen -W '$(_parse_help "$1" --help)' -- "$cur") )
        return
    fi

    _filedir
} &&
complete -F _f3d f3d
