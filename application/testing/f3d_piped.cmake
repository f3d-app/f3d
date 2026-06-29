cmake_minimum_required(VERSION 3.19)

separate_arguments(F3D_ARGS NATIVE_COMMAND ${F3D_ARGS})

# By default the piped data is passed as the input model through the `-` filename.
# When F3D_PIPED_NO_FILENAME is set, no filename is provided and the data is expected
# to be consumed through an option such as `--load-statefile=-`.
if(F3D_PIPED_NO_FILENAME)
  set(F3D_PIPED_FILENAME_ARG)
else()
  set(F3D_PIPED_FILENAME_ARG -)
endif()

execute_process(
  COMMAND ${F3D_EXE} ${F3D_PIPED_FILENAME_ARG} ${F3D_ARGS}
  INPUT_FILE ${F3D_PIPED_DATA}
  COMMAND_ERROR_IS_FATAL ANY
  )
