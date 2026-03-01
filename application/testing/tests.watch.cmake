## Custom test for watch CLI option
# Watch testing require GLX rendering on Linux
if(F3D_MODULE_DMON AND (WIN32 OR APPLE OR F3D_TESTING_ENABLE_GLX_TESTS))
  if(UNIX)
    set(_f3d_os_script_ext "sh")
    set(_f3d_os_script_exec "")
  elseif(WIN32)
    set(_f3d_os_script_ext "ps1")
    find_program(_f3d_os_script_exec NAMES pwsh powershell)
  endif()
  # Custom bash/pwsh test for testing watch option
  add_test (NAME f3d::TestWatch COMMAND ${_f3d_os_script_exec} ${CMAKE_CURRENT_SOURCE_DIR}/test_watch.${_f3d_os_script_ext} $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data ${CMAKE_BINARY_DIR}/Testing/Temporary)
  set_tests_properties(f3d::TestWatch PROPERTIES RUN_SERIAL TRUE TIMEOUT 90)
endif()
