# F3D Installation

include(GNUInstallDirs)

install(TARGETS f3d
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ".")

file(GLOB data_files LIST_DIRECTORIES FALSE "${CMAKE_SOURCE_DIR}/data/*")
install(FILES ${data_files}
  DESTINATION data
  COMPONENT "Examples")

install(FILES LICENSE README.md
  DESTINATION "."
  COMPONENT "Documentation")
