find_package(Python3 COMPONENTS Interpreter REQUIRED)

if (BUILD_TESTING)
  if (NOT F3D_EXAMPLES_EXTERNAL_QT)
    set(PYTEST_K_EXPR "-k \"not test_minimal_qt6\"")
  endif()

  add_test(
    NAME test_python_examples
    COMMAND ${Python3_EXECUTABLE} -m pytest ${PYTEST_K_EXPR} ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()
