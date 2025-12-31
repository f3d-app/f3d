cmake_minimum_required(VERSION 3.19)

string(REPLACE " " ";" F3D_ARGS ${F3D_ARGS})
execute_process(COMMAND ${F3D_EXE} - ${F3D_ARGS} --verbose
INPUT_FILE ${F3D_PIPED_DATA}
COMMAND_ERROR_IS_FATAL ANY
)

