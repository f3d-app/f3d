#[==[
@file f3dOptions.cmake

This module contains all the methods required to parse a JSON file specifying options
and generate the associated CXX code.
#]==]

#[==[
@brief generate public and private headers for a provided options.json

~~~
f3d_generate_options(
  INPUT_JSON "path/to/options.json"
  INPUT_PUBLIC_HEADER "path/to/options.h.in"
  INPUT_PRIVATE_HEADER "path/to/options_tools.h.in"
  DESTINATION "path/to/destination/folder"
  OUTPUT_NAME "options"
	)
~~~

#]==]

function (f3d_generate_options)
  cmake_parse_arguments(PARSE_ARGV 0 _f3d_generate_options
    ""
    "INPUT_JSON;INPUT_PUBLIC_HEADER;INPUT_PRIVATE_HEADER;DESTINATION;OUTPUT_NAME"
    "")

  if (_f3d_generate_options_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unparsed arguments for f3d_generate_options: "
      "${_f3d_generate_options_UNPARSED_ARGUMENTS}")
  endif ()

  if (NOT DEFINED _f3d_generate_options_INPUT_JSON)
    message(FATAL_ERROR
      "Missing INPUT_JSON argument for f3d_generate_options")
  endif ()

  if (NOT DEFINED _f3d_generate_options_INPUT_PUBLIC_HEADER)
    message(FATAL_ERROR
      "Missing INPUT_PUBLIC_HEADER argument for f3d_generate_options")
  endif ()

  if (NOT DEFINED _f3d_generate_options_INPUT_PRIVATE_HEADER)
    message(FATAL_ERROR
      "Missing INPUT_PRIVATE_HEADER argument for f3d_generate_options")
  endif ()

  if (NOT DEFINED _f3d_generate_options_DESTINATION)
    message(FATAL_ERROR
      "Missing DESTINATION argument for f3d_generate_options")
  endif ()

  if (NOT DEFINED _f3d_generate_options_OUTPUT_NAME)
    message(FATAL_ERROR
      "Missing OUTPUT_NAME argument for f3d_generate_options")
  endif ()


  # Parse options.json and generate headers
  set(_option_basename "")
  set(_option_indent "")

  # Add a configure depends on the input file
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_f3d_generate_options_INPUT_JSON})

  ## Read the .json file and complete the struct
  file(READ ${_f3d_generate_options_INPUT_JSON} _options_json)
  _parse_json_option(${_options_json})

  list(JOIN _options_setter ";\n    else " _options_setter)
  list(JOIN _options_getter ";\n  else " _options_getter)
  list(JOIN _options_string_setter ";\n  else " _options_string_setter)
  list(JOIN _options_string_getter ";\n  else " _options_string_getter)
  list(JOIN _options_lister ",\n  " _options_lister)

  configure_file(
    "${_f3d_generate_options_INPUT_PUBLIC_HEADER}"
    "${_f3d_generate_options_DESTINATION}/public/${_f3d_generate_options_OUTPUT_NAME}.h")

  configure_file(
    "${_f3d_generate_options_INPUT_PRIVATE_HEADER}"
    "${_f3d_generate_options_DESTINATION}/private/${_f3d_generate_options_OUTPUT_NAME}_tools.h")

endfunction()

#[==[
@brief internal recursive method use to parse json structure and generate variables
#]==]
function(_parse_json_option _top_json)
  # TODO Add a deprecation mechanism
  string(JSON _options_length LENGTH ${_top_json})
  MATH(EXPR _options_length "${_options_length} - 1")
  foreach(_json_idx RANGE ${_options_length})
    # Recover the json element name
    string(JSON _member_name MEMBER ${_top_json} ${_json_idx})
    # Read the json element
    string(JSON _cur_json GET ${_top_json} ${_member_name})
    # Recover its type and default if it is a leaf option
    string(JSON _option_type ERROR_VARIABLE _option_type_error GET ${_cur_json} "type")
    string(JSON _option_type_type ERROR_VARIABLE _option_type_type_error TYPE ${_cur_json} "type")
    if(_option_type_error STREQUAL "NOTFOUND" AND ${_option_type_type} STREQUAL "STRING")
       # Leaf option found!

       # Recover default_value if any
       string(JSON _option_default_value ERROR_VARIABLE _default_value_error GET ${_cur_json} "default_value")

       set(_option_name "${_option_basename}${_member_name}")

       # Identify types
       set(_option_actual_type ${_option_type})
       set(_option_variant_type ${_option_type})
       set(_option_default_value_start "")
       set(_option_default_value_end "")

       if(_option_type STREQUAL "double_vector")
         set(_option_actual_type "std::vector<double>")
         set(_option_variant_type "std::vector<double>")
         set(_option_default_value_start "{")
         set(_option_default_value_end "}")
       elseif(_option_type STREQUAL "string")
         set(_option_actual_type "std::string")
         set(_option_variant_type "std::string")
         set(_option_default_value_start "\"")
         set(_option_default_value_end "\"")
       elseif(_option_type STREQUAL "ratio")
         set(_option_actual_type "f3d::ratio_t")
         set(_option_variant_type "double")
       endif()

       # Add option to struct and methods

       if(_default_value_error STREQUAL "NOTFOUND")
         # Use default_value
         string(APPEND _options_struct "${_option_indent}  ${_option_actual_type} ${_member_name} = ${_option_default_value_start}${_option_default_value}${_option_default_value_end};\n")
         set(_optional_getter "")
       else()
         # No default_value, it is an std::optional
         string(APPEND _options_struct "${_option_indent}  std::optional<${_option_actual_type}> ${_member_name};\n")
         set(_optional_getter ".value()")
       endif()

       list(APPEND _options_setter "if (name == \"${_option_name}\") opt.${_option_name} = std::get<${_option_variant_type}>(value)")
       list(APPEND _options_getter "if (name == \"${_option_name}\") return opt.${_option_name}${_optional_getter}")
       list(APPEND _options_string_setter "if (name == \"${_option_name}\") opt.${_option_name} = options_tools::parse<${_option_actual_type}>(str)")
       list(APPEND _options_string_getter "if (name == \"${_option_name}\") return options_tools::format(opt.${_option_name}${_optional_getter})")
       list(APPEND _options_lister "\"${_option_name}\"")

    else()
      # Group found, add in the struct and recurse
      set(_option_prevname ${_option_basename})
      set(_option_previndent ${_option_indent})
      string(APPEND _option_indent "  ")
      string(APPEND _option_basename "${_member_name}.")
      string(APPEND _options_struct "${_option_indent}struct ${_member_name} {\n")
      _parse_json_option(${_cur_json})
      string(APPEND _options_struct "${_option_indent}} ${_member_name};\n\n")
      set(_option_indent ${_option_previndent})
      set(_option_basename ${_option_prevname})
    endif()
  endforeach()
  # Set appended variables and list in the parent before leaving the recursion
  # Always use quotes for string variable as it contains semi-colons
  set(_options_struct "${_options_struct}" PARENT_SCOPE)
  set(_options_setter ${_options_setter} PARENT_SCOPE)
  set(_options_getter ${_options_getter} PARENT_SCOPE)
  set(_options_string_setter ${_options_string_setter} PARENT_SCOPE)
  set(_options_string_getter ${_options_string_getter} PARENT_SCOPE)
  set(_options_lister ${_options_lister} PARENT_SCOPE)
endfunction()
