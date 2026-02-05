# ==============================================================================
# f3dCLIOptions.cmake
#
# This script parses cli-options.json and generates F3DCLIOptions.h,
# completions.bash, completions.fish, and completions.zsh.
#
# Invoked at COMPILE TIME via:
#   cmake -DF3D_SOURCE_DIR=<path> -DF3D_BINARY_DIR=<path>
#         -DENABLED_CONDITIONALS=<list> -P f3dCLIOptions.cmake
#
# F3D_SOURCE_DIR is the root src directory of the F3D project.
#
# F3D_BINARY_DIR is the root of the binary directory for the F3D project.
#
# ENABLED_CONDITIONALS is a semicolon-separated list of the macro names that
# are enabled at configure time (e.g. "F3D_MODULE_RAYTRACING;F3D_MODULE_DMON").
# Any group or option in the JSON whose "conditional" field names a macro that
# is NOT in this list is silently dropped. Any group or option whose
# "conditional" IS in the list is emitted unconditionally (no #if guards).
# The array size is therefore a single compile-time constant — no preprocessor
# branching is needed in the generated header.
# ==============================================================================

# ------------------------------------------------------------------------------
# Validate inputs
# ------------------------------------------------------------------------------
if(NOT F3D_SOURCE_DIR)
  message(FATAL_ERROR "f3dCLIOptions.cmake: F3D_SOURCE_DIR not provided")
endif()
if(NOT F3D_BINARY_DIR)
  message(FATAL_ERROR "f3dCLIOptions.cmake: F3D_BINARY_DIR not provided")
endif()
# ENABLED_CONDITIONALS is allowed to be empty (no optional features enabled).

# ------------------------------------------------------------------------------
# Read the JSON blob
# ------------------------------------------------------------------------------
file(READ ${F3D_SOURCE_DIR}/resources/cli-options.json CLI_OPTIONS_JSON)
string(JSON NUM_GROUPS LENGTH "${CLI_OPTIONS_JSON}" "groups")

# ------------------------------------------------------------------------------
# Helper: escape a string for use inside C++ "..." literals.
# CMake's string(JSON GET ...) returns the *unescaped* value (no surrounding
# quotes, and escape sequences already decoded to literal characters).
# We need to re-escape backslashes and double-quotes for C++.
#   _raw_str - the raw JSON string without escape sequences
#   OUT_VAR - the output string with proper escape sequences
# ------------------------------------------------------------------------------
function(_escape_for_cpp _raw_str OUT_VAR)
  # Order matters: escape backslashes first, then quotes
  string(REPLACE "\\" "\\\\" _temp "${_raw_str}")
  string(REPLACE "\"" "\\\"" _result "${_temp}")
  set(${OUT_VAR} "${_result}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# Helper: decide whether a conditional value from the JSON is satisfied.
#   _cond   – the macro name read from JSON (empty string = no condition)
#   OUT_VAR – set to TRUE if the item should be included, FALSE otherwise
# ------------------------------------------------------------------------------
# Policy need to use IN_LIST operator below
cmake_policy(SET CMP0057 NEW)
function(_conditional_satisfied _cond OUT_VAR)
  if(_cond STREQUAL "")
    # No conditional -> always included
    set(${OUT_VAR} TRUE PARENT_SCOPE)
  elseif(_cond IN_LIST ENABLED_CONDITIONALS)
    # Conditional is in the enabled set -> included
    set(${OUT_VAR} TRUE PARENT_SCOPE)
  else()
    # Conditional is NOT enabled -> drop
    set(${OUT_VAR} FALSE PARENT_SCOPE)
  endif()
endfunction()

# ------------------------------------------------------------------------------
# First pass – walk every group, decide include/exclude, count surviving groups
# so we know the exact array size.
# ------------------------------------------------------------------------------
set(_group_count 0)     # number of groups that survive
set(_surviving_indices) # JSON indices of groups that survive
set(_surviving_names)   # their escaped names

math(EXPR _last_group "${NUM_GROUPS} - 1")
# Loop over groups
foreach(_gi RANGE ${_last_group})
  # Read group conditional, if it exists
  string(JSON _gcond ERROR_VARIABLE _group_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "conditional")
  if(_group_json_error) # _group_json_error will be set if "conditional" is not present
    set(_gcond "")
  endif()

  _conditional_satisfied("${_gcond}" _keep)
  if(NOT _keep)
    continue() # entire group is disabled – skip
  endif()

  # Group survives
  string(JSON _name GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "name")

  math(EXPR _group_count "${_group_count} + 1")
  list(APPEND _surviving_indices "${_gi}")
  list(APPEND _surviving_names   "${_name}")
endforeach()

# ------------------------------------------------------------------------------
# Second pass – generate file content
# ------------------------------------------------------------------------------
set(F3D_CLI_OPTIONS_CPP "static inline const std::array<CLIGroup, ${_group_count}> CLIOptions = {{\n")
set(F3D_CLI_OPTIONS_BASH)
set(F3D_CLI_OPTIONS_FISH)
set(F3D_CLI_OPTIONS_ZSH)

set(_option_count 0) # number of options that survive

list(LENGTH _surviving_indices _num_surviving)
math(EXPR _last_surv "${_num_surviving} - 1")
# Loop over surviving groups
foreach(_si RANGE ${_last_surv})
  list(GET _surviving_indices ${_si} _gi)
  list(GET _surviving_names ${_si} _gname)

  string(APPEND F3D_CLI_OPTIONS_CPP "  { \"${_gname}\",\n")
  string(APPEND F3D_CLI_OPTIONS_CPP "    {\n")

  string(JSON _num_options LENGTH "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options")
  math(EXPR _last_opt "${_num_options} - 1")
  # Loop over options
  foreach(_oi RANGE ${_last_opt})
    string(JSON _ocond ERROR_VARIABLE _opt_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "conditional")
    if(_opt_json_error) # _opt_json_error will be set if "conditional" is not present
      set(_ocond "")
    endif()

    _conditional_satisfied("${_ocond}" _keep_opt)
    if(NOT _keep_opt)
      continue() # option is disabled – skip
    endif()

    math(EXPR _option_count "${_option_count} + 1")

    string(JSON _raw_long_name ERROR_VARIABLE _long_name_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "longName")
    _escape_for_cpp("${_raw_long_name}" _long_name)
    string(JSON _raw_short_name ERROR_VARIABLE _short_name_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "shortName")
    _escape_for_cpp("${_raw_short_name}" _short_name)
    string(JSON _raw_help_text ERROR_VARIABLE _help_text_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "helpText")
    _escape_for_cpp("${_raw_help_text}" _help_text)
    string(JSON _raw_value_helper ERROR_VARIABLE _value_helper_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "valueHelper")
    _escape_for_cpp("${_raw_value_helper}" _value_helper)
    string(JSON _raw_implicit_value ERROR_VARIABLE _implicit_value_json_error GET "${CLI_OPTIONS_JSON}" "groups" ${_gi} "options" ${_oi} "implicitValue")
    _escape_for_cpp("${_raw_implicit_value}" _implicit_value)

    # use the `*_json_error`s as an indicator of whether the key is present for this option
    if(_long_name_json_error)
      message(FATAL_ERROR "An option was found in ${F3D_SOURCE_DIR}/resources/cli-options.json without a long name")
    endif()
    if(_short_name_json_error)
      set(_short_name "")
    endif()
    if(_help_text_json_error)
      message(FATAL_ERROR "An option was found in ${F3D_SOURCE_DIR}/resources/cli-options.json without help text")
    endif()
    if(_value_helper_json_error)
      set(_value_helper "")
    endif()
    if(_implicit_value_json_error)
      set(_implicit_value "")
    endif()

    # generate C++ output for this CLI option
    # input, define, and reset are handled separately in the C++ source code
    if(NOT "${_long_name}" STREQUAL "input" AND NOT "${_long_name}" STREQUAL "define" AND NOT "${_long_name}" STREQUAL "reset")
      string(APPEND F3D_CLI_OPTIONS_CPP
        "      { \"${_long_name}\", \"${_short_name}\", \"${_help_text}\", \"${_value_helper}\", \"${_implicit_value}\" },\n")
    endif()

    # generate completion file content for this CLI option
    if(F3D_CLI_OPTIONS_BASH)
      string(APPEND F3D_CLI_OPTIONS_BASH " ") # space separates completion options in bash array
    endif()
    if(F3D_CLI_OPTIONS_FISH)
      string(APPEND F3D_CLI_OPTIONS_FISH "\n") # newline separates fish completion commands
    endif()
    if(F3D_CLI_OPTIONS_ZSH)
      string(APPEND F3D_CLI_OPTIONS_ZSH "\n") # newline separates completion options in zsh array
    endif()
    if(_long_name AND _short_name)
      string(APPEND F3D_CLI_OPTIONS_BASH "--${_long_name} -${_short_name}")
      string(APPEND F3D_CLI_OPTIONS_FISH "complete -c f3d -l ${_long_name} -s ${_short_name} -d \"${_help_text}\"")
      string(APPEND F3D_CLI_OPTIONS_ZSH "  '(-${_short_name} --${_long_name})'{-${_short_name},--${_long_name}}'[${_help_text}]'")
    else() # _long_name must be set (checked earlier)
      string(APPEND F3D_CLI_OPTIONS_BASH "--${_long_name}")
      string(APPEND F3D_CLI_OPTIONS_FISH "complete -c f3d -l ${_long_name} -d \"${_help_text}\"")
      string(APPEND F3D_CLI_OPTIONS_ZSH "  '--${_long_name}[${_help_text}]'")
    endif()

  endforeach()

  string(APPEND F3D_CLI_OPTIONS_CPP "    }\n")
  string(APPEND F3D_CLI_OPTIONS_CPP "  },\n")
endforeach()

string(APPEND F3D_CLI_OPTIONS_CPP "}};")

if(_option_count EQUAL 0)
  message(FATAL_ERROR "No CLI options were found in ${F3D_SOURCE_DIR}/resources/cli-options.json")
endif()

# ------------------------------------------------------------------------------
# Generate F3DCLIOptions.h, completion.bash, completion.fish, and completion.zsh
# ------------------------------------------------------------------------------
configure_file(${F3D_SOURCE_DIR}/application/F3DCLIOptions.h.in
  ${F3D_BINARY_DIR}/application/F3DCLIOptions.h
  @ONLY)
configure_file(${F3D_SOURCE_DIR}/resources/completion.bash.in
  ${F3D_BINARY_DIR}/completion.bash
  @ONLY)
configure_file(${F3D_SOURCE_DIR}/resources/completion.fish.in
  ${F3D_BINARY_DIR}/completion.fish
  @ONLY)
configure_file(${F3D_SOURCE_DIR}/resources/completion.zsh.in
  ${F3D_BINARY_DIR}/completion.zsh
  @ONLY)
