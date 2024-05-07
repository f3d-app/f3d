def fix_line(md_line, cli_flags)
    if m = md_line.strip.match(/(##+) *(`[^\()]+) *(\(.+\))?/)
        h_tag = "h#{m[1].length}"
        flags_md = m[2]
        details = m[3]
        
        # extract flags and use longest one as anchor id
        flag_list = flags_md.scan(/(-+[^=`]+)/).map{|m| m[0]}
        anchor = flag_list.max_by(&:length)
        flag_list.each do |f|
            cli_flags[f] = anchor
        end

        # change `a=b` to `a`_`=b`_ to make the value hint italics so we can style it later
        flags_md = flags_md.gsub(/=([^`]+)`/, '`_`=\1`_')
        flags_html = yield(flags_md)

        details_html = if details then " <small>#{yield(details)}</small>" else "" end

        return "<#{h_tag} id=\"#{anchor}\" class=\"cli\">#{flags_html}#{details_html}</#{h_tag}>"
    else
        return md_line
    end
end


Jekyll::Hooks.register :pages, :pre_render do |page|
    if page.name == 'OPTIONS.md'
        converter = page.site.find_converter_instance(::Jekyll::Converters::Markdown)
        cli_flags = {}
        page.content = page.content.lines.map{ |line| fix_line(line, cli_flags){|md|
            converter.convert(md).strip.delete_prefix('<p>').delete_suffix('</p>')}
        }.join
        
        # add anchor links to all known cli flags occurences
        page.content = page.content.gsub(/`(--?[^=`]+)(=[^`]*)?`/){|m|
            if cli_flags.include? $1 then "[`#{$1}#{$2}`](##{cli_flags[$1]})" else m end
        }
    end
end
