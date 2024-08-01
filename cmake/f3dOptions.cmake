#[==[
@file f3dOptions.cmake

This module contains all the methods required to parse a JSON file specifying options
and generate the associated CXX code.
#]==]

#[==[
@brief generate public and private headers for a provided options.json

~~~
f3d_generate_options(
  INPUT "path/to/options.json"
  DESTINATION "path/to/destination/folder"
  [NAME "optional_prefix"]
	)
~~~

#]==]

function (f3d_generate_options)
  cmake_parse_arguments(PARSE_ARGV 0 _f3d_generate_options
    ""
    "INPUT;DESTINATION;NAME"
    "")

  if (_f3d_generate_options_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unparsed arguments for f3d_generate_options: "
      "${_f3d_generate_options_UNPARSED_ARGUMENTS}")
  endif ()

  if (NOT DEFINED _f3d_generate_options_INPUT)
    message(FATAL_ERROR
      "Missing INPUT argument for f3d_generate_options")
  endif ()

  if (NOT DEFINED _f3d_generate_options_DESTINATION)
    message(FATAL_ERROR
      "Missing DESTINATION argument for f3d_generate_options")
  endif ()

  set(_f3d_generate_options_prefix "")
  if (DEFINED _f3d_generate_options_NAME)
    set(_f3d_generate_options_prefix ${_f3d_generate_options_NAME}_)
  endif ()

  # Parse options.json and generate headers
  set(_option_basename "")
  set(_option_indent "")

  # Add a configure depends on the input file
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_f3d_generate_options_INPUT})

  ## Read the .json file and complete the struct
  file(READ ${_f3d_generate_options_INPUT} _options_json)
  _parse_json_option(${_options_json})

  file(WRITE ${_f3d_generate_options_DESTINATION}/public/options_struct.h "\
#ifndef f3d_${_f3d_generate_options_prefix}options_struct_h
#define f3d_${_f3d_generate_options_prefix}options_struct_h

#include \"types.h\"

#include <vector>
#include <string>

namespace f3d {
struct ${_f3d_generate_options_prefix}options_struct {
${_options_struct}};
};
#endif
")

  list(JOIN _options_setter ";\n    else " _options_setter)
  list(JOIN _options_getter ";\n  else " _options_getter)
  list(JOIN _options_typer ";\n  else " _options_typer)
  list(JOIN _options_lister ",\n  " _options_lister)
  file(WRITE ${_f3d_generate_options_DESTINATION}/private/options_struct_internals.h "\
#ifndef f3d${_f3d_generate_options_prefix}_options_struct_internals_h
#define f3d${_f3d_generate_options_prefix}_options_struct_internals_h

#include \"options_struct.h\"
#include \"options.h\"

namespace ${_f3d_generate_options_prefix}options_struct_internals {
void set(f3d::options_struct& ostruct, const std::string& name, const option_variant_t& value){
  try
  {
    ${_options_setter};
    else throw f3d::options::inexistent_exception(\"Option \" + name + \" does not exist\");
  }
  catch (const std::bad_variant_access&)
  {
    throw f3d::options::incompatible_exception(
      \"Trying to set \" + name + \" with incompatible type\");
  }
}

option_variant_t get(const f3d::options_struct& ostruct, const std::string& name){
  option_variant_t var;
  ${_options_getter};
  else throw f3d::options::inexistent_exception(\"Option \" + name + \" does not exist\");
  return var;
}

option_types getType(const std::string& name){
  ${_options_typer};
  else throw f3d::options::inexistent_exception(\"Option \" + name + \" does not exist\");
  // Cannot be reached
  return option_types::_bool;
}

std::vector<std::string> getNames() {
  std::vector<std::string> vec{
  ${_options_lister}
  };
  return vec;
}
}
#endif
")

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
         list(APPEND _options_setter "if (name == \"${_option_name}\") ostruct.${_option_name} = std::get<std::vector<double>>(value)")
       elseif(_option_type STREQUAL "string")
         string(APPEND _options_struct "${_option_indent}  std::string ${_member_name} = \"${_option_default_value}\";\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") ostruct.${_option_name} = std::get<std::string>(value)")
       elseif(_option_type STREQUAL "ratio")
         string(APPEND _options_struct "${_option_indent} f3d::ratio_t ${_member_name} = f3d::ratio_t(${_option_default_value});\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") ostruct.${_option_name} = std::get<double>(value)")
       else()
         string(APPEND _options_struct "${_option_indent}  ${_option_type} ${_member_name} = ${_option_default_value};\n")
         list(APPEND _options_setter "if (name == \"${_option_name}\") ostruct.${_option_name} = std::get<${_option_type}>(value)")
       endif()
       list(APPEND _options_getter "if (name == \"${_option_name}\") var = ostruct.${_option_name}")
       list(APPEND _options_typer "if (name == \"${_option_name}\") return option_types::_${_option_type}")
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
  set(_options_typer ${_options_typer} PARENT_SCOPE)
  set(_options_lister ${_options_lister} PARENT_SCOPE)
endfunction()
