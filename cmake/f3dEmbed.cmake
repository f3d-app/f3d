# Modified from vtkEncodeString (BSD licensed)

set(_embed_script_file "${CMAKE_CURRENT_LIST_FILE}")

function (f3d_embed_file)
  cmake_parse_arguments(PARSE_ARGV 0 _embed
    ""
    "INPUT;NAME"
    "")

  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${_embed_NAME}.h"
            "${CMAKE_CURRENT_BINARY_DIR}/${_embed_NAME}.cxx"
    DEPENDS "${_embed_script_file}"
            "${_embed_INPUT}"
    COMMAND "${CMAKE_COMMAND}"
            "-Dbinary_dir=${CMAKE_CURRENT_BINARY_DIR}"
            "-Dsource_file=${_embed_INPUT}"
            "-Doutput_name=${_embed_NAME}"
            "-D_embed_run=ON"
            -P "${_embed_script_file}")
endfunction ()

if (_embed_run AND CMAKE_SCRIPT_MODE_FILE)
  set(output_header "${binary_dir}/${output_name}.h")
  set(output_source "${binary_dir}/${output_name}.cxx")

  file(WRITE "${output_header}" "")
  file(WRITE "${output_source}" "")

  file(APPEND "${output_header}"
    "#ifndef ${output_name}_h\n\n#define ${output_name}_h\n\n")

  # Read the file in HEX format
  file(READ "${source_file}" original_content HEX)

  string(LENGTH "${original_content}" output_size)
  math(EXPR output_size "${output_size} / 2")

  file(APPEND "${output_header}"
  "extern const unsigned char ${output_name}[${output_size}];\n\n#endif\n")

  file(APPEND "${output_source}"
    "#include \"${output_name}.h\"\n\nconst unsigned char ${output_name}[${output_size}] = {\n")
  string(REGEX REPLACE "\([0-9a-f][0-9a-f]\)" ",0x\\1" escaped_content "${original_content}")
  # Hard line wrap the file.
  string(REGEX REPLACE "\(...............................................................................,\)" "\\1\n" escaped_content "${escaped_content}")

  # Remove the leading comma.
  string(REGEX REPLACE "^," "" escaped_content "${escaped_content}")

  file(APPEND "${output_source}"
      "${escaped_content}\n")
  file(APPEND "${output_source}"
      "};\n")
endif ()
