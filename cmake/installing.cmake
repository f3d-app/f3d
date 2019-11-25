# F3D Installation

file(GLOB data_files LIST_DIRECTORIES FALSE "${CMAKE_SOURCE_DIR}/data/*")
install(FILES ${data_files}
  DESTINATION data
  COMPONENT "Examples")

install(FILES LICENSE README.md
  DESTINATION "."
  COMPONENT "Documentation")
