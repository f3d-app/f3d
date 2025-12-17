cmake_minimum_required(VERSION 3.10)

# interactive-app
add_test(NAME test_interactive-app-java COMMAND ${Java_JAVA_EXECUTABLE} -jar "$<TARGET_PROPERTY:interactive-app-java,JAR_FILE>" "${CMAKE_CURRENT_SOURCE_DIR}/data/cow.vtp" 1)
