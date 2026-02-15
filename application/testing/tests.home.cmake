## Test behaviors related to HOME directory and similar
if(NOT WIN32)
  f3d_application_test(NAME TestHDRINoCachePath ENV "HOME=" DATA dragon.vtu ARGS -f --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr REGEXP "Cannot cache .* as no cache path has been set." NO_BASELINE LONG_TIMEOUT)

  add_test(NAME f3d::TestHOMEOutput COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=~/Testing/Temporary/TestHOMEOutput.png --resolution=300,300 --no-config)
  set_tests_properties(f3d::TestHOMEOutput PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")
  set_tests_properties(f3d::TestHOMEOutput PROPERTIES FIXTURES_SETUP f3d::TestHOMEOutput_FIXTURE)

  add_test(NAME f3d::TestHOMEReference COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=${CMAKE_BINARY_DIR}/Testing/Temporary/TestHOMEReference.png --reference=~/Testing/Temporary/TestHOMEOutput.png --resolution=300,300 --no-config)
  set_tests_properties(f3d::TestHOMEReference PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")
  set_tests_properties(f3d::TestHOMEReference PROPERTIES FIXTURES_REQUIRED f3d::TestHOMEOutput_FIXTURE)

  add_test(NAME f3d::TestHOMEOutputTemplate COMMAND $<TARGET_FILE:f3d> ${F3D_SOURCE_DIR}/testing/data/suzanne.stl --output=~/Testing/Temporary/{model}{n}.png --resolution=300,300 --verbose)
  set_tests_properties(f3d::TestHOMEOutputTemplate PROPERTIES ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}")

  f3d_application_test(NAME TestHOMEInput ARGS --input=~/testing/data/suzanne.stl ENV "HOME=${F3D_SOURCE_DIR}")
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestHOMEConfig DATA suzanne.stl CONFIG ~/testing/configs/complex.json ENV "HOME=${F3D_SOURCE_DIR}" UI)
  endif()
  f3d_application_test(NAME TestHOMEPlugin ARGS --load-plugins=~/testing/data/invalid.so REGEXP "Cannot open the library" NO_BASELINE ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_application_test(NAME TestHOMEInteractionRecord DATA cow.vtp ARGS --interaction-test-record=~/Testing/Temporary/TestHOMEInteractionRecord.log NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_application_test(NAME TestHOMEInteractionPlay DATA cow.vtp ARGS --interaction-test-play=~/Testing/Temporary/TestHOMEInteractionRecord.log DEPENDS TestHOMEInteractionRecord NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_application_test(NAME TestHOMEColorMapFile DATA dragon.vtu ARGS --colormap-file=~/testing/data/viridis8.png --scalar-coloring --coloring-component=1 ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_application_test(NAME TestHOMEScreenshot DATA suzanne.ply ARGS --screenshot-filename=~/Testing/Temporary/TestHOMEScreenshot.png --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log REGEXP "saving screenshot to ${CMAKE_BINARY_DIR}/Testing/Temporary/TestHOMEScreenshot.png" NO_BASELINE ENV "HOME=${CMAKE_BINARY_DIR}")
  f3d_application_test(NAME TestHOMECommandScript DATA suzanne.ply ARGS --command-script=~/testing/scripts/TestHOMECommandScript.txt REGEXP "Camera focal point" NO_BASELINE ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_application_test(NAME TestHOMEFontFile DATA suzanne.stl ARGS -n --font-file=~/testing/data/Crosterian.ttf ENV "HOME=${F3D_SOURCE_DIR}" UI)
  f3d_application_test(NAME TestHOMETexture DATA suzanne.ply ARGS --texture-matcap=~/testing/data/skin.png ENV "HOME=${F3D_SOURCE_DIR}")
  f3d_application_test(NAME TestHOMEHDRI DATA suzanne.stl ARGS --hdri-file=~/testing/data/palermo_park_1k.hdr --hdri-ambient --hdri-skybox ENV "HOME=${F3D_SOURCE_DIR}" LONG_TIMEOUT)
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_application_test(NAME TestHOMEInteractionDropHDRICollapse INTERACTION_CONFIGURE ENV "HOME=${F3D_SOURCE_DIR}" LONG_TIMEOUT UI) #X;DropEvent dragon.vtu;DropEvent palermo.hdr;
  endif()

  if(NOT APPLE)

    # This tests the XDG_CONFIG_HOME is taken into account AND that config order between locations is respected
    file(COPY "${F3D_SOURCE_DIR}/testing/configs/02_home.json" DESTINATION "${CMAKE_BINARY_DIR}/share/a_home_config/f3d/config_build.d")
    f3d_application_test(NAME TestXDG_CONFIG_HOME DATA suzanne.ply CONFIG config_build ENV "XDG_CONFIG_HOME=${CMAKE_BINARY_DIR}/share/a_home_config" UI TONE_MAPPING)

    f3d_application_test(NAME TestXDG_CACHE_HOMECoverage DATA suzanne.ply NO_RENDER NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR}")

    # Setting XDG_CACHE_HOME is needed for cache to work when HOME is not set
    f3d_application_test(NAME TestNoHOMEScreenshot DATA suzanne.ply ARGS --screenshot-filename=TestNoHOMEScreenshot.png --interaction-test-play=${F3D_SOURCE_DIR}/testing/recordings/TestScreenshot.log  REGEXP "saving screenshot to ${CMAKE_BINARY_DIR}/application/testing/TestNoHOMEScreenshot.png" NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR};HOME=")
    f3d_application_test(NAME TestNoHOMEConfig DATA suzanne.ply CONFIG config_build REGEXP "Using config directory ${CMAKE_BINARY_DIR}/share/f3d/configs/config_build.d" NO_RENDER NO_BASELINE ENV "XDG_CACHE_HOME=${CMAKE_BINARY_DIR};HOME=")
  endif()

else()
  cmake_path(SET _user_profile_path ${CMAKE_BINARY_DIR}/Testing/Temporary/ùser/🥷)
  cmake_path(NATIVE_PATH _user_profile_path _user_profile_path)
  add_test(NAME f3d::TestSetupWinUserProfile COMMAND ${CMAKE_COMMAND} -E make_directory "${_user_profile_path}/AppData/Local")
  f3d_application_test(NAME TestWinUserProfileNonStandard DATA dragon.vtu ENV "USERPROFILE=${_user_profile_path}" DEPENDS TestSetupWinUserProfile)
endif()
