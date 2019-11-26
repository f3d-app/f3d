# F3D Installation

install(FILES LICENSE README.md
  DESTINATION "."
  COMPONENT "Documentation")

if(UNIX AND NOT APPLE)
  install(FILES "${CMAKE_SOURCE_DIR}/resources/f3d.desktop"
    DESTINATION "share/applications")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo32.png"
    DESTINATION "share/icons/hicolor/32x32/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo48.png"
    DESTINATION "share/icons/hicolor/48x48/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo64.png"
    DESTINATION "share/icons/hicolor/64x64/apps"
    RENAME "f3d.png")
  install(FILES "${CMAKE_SOURCE_DIR}/resources/logo.svg"
    DESTINATION "share/icons/hicolor/scalable/apps"
    RENAME "f3d.svg")
endif()
