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
    string(JSON _option_type ERROR_VARIABLE _type_error GET ${_cur_json} "type")
    string(JSON _option_default_value ERROR_VARIABLE _default_value_error GET ${_cur_json} "default_value")
    if(_type_error STREQUAL "NOTFOUND" AND _default_value_error STREQUAL "NOTFOUND")
       # Leaf option found!
       set(_option_name "${_option_basename}${_member_name}")
       # Add an option in the struct based on its type
       if(_option_type STREQUAL "double_vector")
         string(APPEND _options_struct "${_option_indent}  std::vector<double> ${_member_name} = {${_option_default_value}};\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") opt.${_option_name} = std::get<std::vector<double>>(value)")
       elseif(_option_type STREQUAL "string")
         string(APPEND _options_struct "${_option_indent}  std::string ${_member_name} = \"${_option_default_value}\";\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") opt.${_option_name} = std::get<std::string>(value)")
       elseif(_option_type STREQUAL "ratio")
         string(APPEND _options_struct "${_option_indent} f3d::ratio_t ${_member_name} = f3d::ratio_t(${_option_default_value});\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") opt.${_option_name} = std::get<double>(value)")
       else()
         string(APPEND _options_struct "${_option_indent}  ${_option_type} ${_member_name} = ${_option_default_value};\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") opt.${_option_name} = std::get<${_option_type}>(value)")
       endif()
       list(APPEND _options_getter "if (name == \"${_option_name}\") return opt.${_option_name}")
       list(APPEND _options_string_setter "if (name == \"${_option_name}\") options_tools::parse(str, opt.${_option_name})")
       list(APPEND _options_string_getter "if (name == \"${_option_name}\") return options_tools::toString(opt.${_option_name})")
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
