cmake_minimum_required(VERSION 3.19)

separate_arguments(F3D_ARGS NATIVE_COMMAND ${F3D_ARGS})

# F3D_PIPED_ARG is prepended to the `-` standard input marker. It is empty by default (the piped data
# is the input model, read through `-`) and can be set to an option prefix such as `--load-statefile=`
# so the piped data is consumed by that option (`--load-statefile=-`) rather than as the input model.
execute_process(
  COMMAND ${F3D_EXE} ${F3D_PIPED_ARG}- ${F3D_ARGS}
  INPUT_FILE ${F3D_PIPED_DATA}
  COMMAND_ERROR_IS_FATAL ANY
  )
