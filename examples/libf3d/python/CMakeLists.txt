find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(NAME test_python_examples COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR})
