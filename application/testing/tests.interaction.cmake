## Interaction Tests

f3d_test(NAME TestInteractionPostFX DATA cow.vtp INTERACTION TONE_MAPPING LONG_TIMEOUT) #PQAAAAT
f3d_test(NAME TestInteractionTimer DATA cow.vtp NO_BASELINE INTERACTION UI LONG_TIMEOUT) #Z
f3d_test(NAME TestInteractionTrackball DATA cow.vtp ARGS --interaction-trackball INTERACTION LONG_TIMEOUT) #Mouse movements;
f3d_test(NAME TestInteractionTrackballBind DATA cow.vtp INTERACTION LONG_TIMEOUT) #K#Mouse movements;
f3d_test(NAME TestInteractionCycleCell DATA waveletArrays.vti INTERACTION LONG_TIMEOUT) #VCCC
f3d_test(NAME TestInteractionCycleComp DATA dragon.vtu INTERACTION) #SYYYY
f3d_test(NAME TestInteractionCycleScalars DATA dragon.vtu INTERACTION) #BSSSS
f3d_test(NAME TestInteractionCycleCellInvalidIndex DATA waveletArrays.vti INTERACTION) #SSC
f3d_test(NAME TestInteractionCycleBlending DATA suzanne.ply ARGS --opacity=0.8 INTERACTION LONG_TIMEOUT) #PPPPP # Cycle to ddp
f3d_test(NAME TestInteractionVolumeCycle DATA waveletArrays.vti ARGS INTERACTION) #VSS
f3d_test(NAME TestInteractionVolumeAfterColoring DATA waveletArrays.vti ARGS INTERACTION) #SSSV
f3d_test(NAME TestInteractionVolumeInverse DATA HeadMRVolume.mhd ARGS --camera-position=127.5,-400,127.5 --camera-view-up=0,0,1 INTERACTION THRESHOLD 0.05) #VI #Small rendering differences due to volume rendering
f3d_test(NAME TestInteractionCorrectCameraForVolumeSwitch ARGS --no-config -v DATA dragon.vtu INTERACTION UI) #v
f3d_test(NAME TestInteractionMultiFileVolume DATA multi ARGS --multi-file-mode=all INTERACTION) #SSVB
f3d_test(NAME TestInteractionPointCloud DATA pointsCloud.vtp ARGS --point-sprites-size=20 INTERACTION) #O
f3d_test(NAME TestInteractionDirectory DATA mb INTERACTION ARGS --scalar-coloring) #Right;Right;Right;Left;Up;
f3d_test(NAME TestInteractionDirectoryLoop DATA mb/recursive INTERACTION ARGS --scalar-coloring --filename UI) #Left;Left;Left;Left;Left;
f3d_test(NAME TestInteractionDirectoryEmpty DATA mb INTERACTION NO_DATA_FORCE_RENDER UI) #Right;Right;Right;
f3d_test(NAME TestInteractionDirectoryEmptyVerbose DATA mb ARGS --verbose NO_BASELINE INTERACTION REGEXP "is not a file of a supported file format") #Right;Right;Right;HMCSY
f3d_test(NAME TestInteractionTensorsCycleComp DATA tensors.vti ARGS --scalar-coloring --coloring-component=-2 INTERACTION) #SYYYYYYYYYY
f3d_test(NAME TestInteractionCycleScalarsCompCheck DATA dragon.vtu ARGS -b --scalar-coloring --coloring-component=2 INTERACTION) #S
f3d_test(NAME TestInteractionTAA DATA suzanne.ply ARGS --anti-aliasing=taa INTERACTION) #Render;Render...
f3d_test(NAME TestInteractionTAAMiddleClick DATA suzanne.ply ARGS --anti-aliasing=taa INTERACTION) #Render;Render...;MiddleClick;Render;Render...
f3d_test(NAME TestInteractionCycleVerbose DATA dragon.vtu ARGS --verbose -s NO_BASELINE INTERACTION REGEXP "Not coloring") #SSSSYC
f3d_test(NAME TestInteractionCycleVerboseLevelsUsingBinding DATA dragon.vtu ARGS --verbose=info NO_BASELINE INTERACTION REGEXP "Verbose level changed to: Debug") #Shift+V;Shift+V;Shift+V;Shift+V;Shift+V
f3d_test(NAME TestInteractionVerboseLevelPreservedOnReload DATA dragon.vtu NO_BASELINE INTERACTION REGEXP "Not coloring") #Shift+V;Shift+V;Shift+V;Shift+V;Up
f3d_test(NAME TestInteractionLightIntensity DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionMultiFileColoring DATA mb/recursive ARGS --multi-file-mode=all INTERACTION) #SSSB
f3d_test(NAME TestInteractionOpacity DATA dragon.vtu INTERACTION) # Shift+P * 21; Ctrl+P * 5
f3d_test(NAME TestInteractionReload DATA dragon.vtu ARGS -e INTERACTION) #Up;
f3d_test(NAME TestInteractionLoadParentDirectory DATA multi/dragon.vtu ARGS --filename INTERACTION UI) #Down;
f3d_test(NAME TestInteractionEmptyLoadParentDirectory INTERACTION NO_BASELINE REGEXP "No files loaded, no rendering performed") #Down;
f3d_test(NAME TestInteractionMultiFileLoadParentDirectory DATA mb/mb_0_0.vtu ARGS --multi-file-mode=all --filename INTERACTION UI) #Down;
f3d_test(NAME TestInteractionInvertZoom DATA suzanne.ply ARGS --invert-zoom INTERACTION)
f3d_test(NAME TestInteractionSimpleExit DATA cow.vtp REGEXP "Interactor has been stopped" INTERACTION NO_BASELINE) #CTRL+Q

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionActors DATA cow.vtp INTERACTION UI) #EXGMN
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231102)
  f3d_test(NAME TestInteractionPointCloudTAA DATA pointsCloud.vtp ARGS --anti-aliasing=taa -o --point-sprites-size=20 INTERACTION) #Render;Render...
endif()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12098
if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250504)
  f3d_test(NAME TestInteractionStochasticTAA DATA alpha.glb ARGS --anti-aliasing=taa --blending=stochastic INTERACTION) #Render;Render...
  f3d_test(NAME TestInteractionGaussianStochasticTAA DATA small.splat ARGS -sy --point-sprites=gaussian --point-sprites-absolute-size --point-sprites-size=1 --blending=stochastic --anti-aliasing=taa --camera-position=-1.65,-0.06,1.96 --camera-focal-point=-1.65,1.24,1.96 --camera-view-up=0.9954,0,0.0955 INTERACTION LONG_TIMEOUT) #Render;Render...
  f3d_test(NAME TestInteractionAndCLIBlending ARGS --blending DATA suzanne.stl alpha.glb INTERACTION) #PP;Right
endif()

# Using gaussian splatting require > 9.3 to not warn
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  f3d_test(NAME TestInteractionConfigFileAndCommand DATA multi ARGS -o CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI LONG_TIMEOUT) #OX;Right;N;Right;Right;Right
endif()

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
  f3d_test(NAME TestInteractionAxesGridToggle INTERACTION DATA suzanne.ply THRESHOLD 0.08) #Shift+x
endif()

if (F3D_MODULE_TINYFILEDIALOGS)
  f3d_test(NAME TestInteractionFileOpen INTERACTION NO_BASELINE ENV CTEST_OPEN_DIALOG_FILE=${F3D_SOURCE_DIR}/testing/data/cow.vtp)
endif ()

if(F3D_MODULE_DMON)
  f3d_test(NAME TestInteractionVerboseWatchUnwatch ARGS --watch --verbose DATA cow.vtp multi/dragon.vtu NO_BASELINE INTERACTION REGEXP "Stopped watching") #Right
endif()

## Config
f3d_test(NAME TestInteractionConfigFileMulti DATA multi CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #SY;Right;XG;Right;N;Right;Right

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionConfigFileNoColorBar DATA multi CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #Right;Right;Left
  f3d_test(NAME TestInteractionConfigFileBindings DATA dragon.vtu CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #Ctrl+Shift+O;Ctrl+O;Shift+O;O;3
  f3d_test(NAME TestInteractionConfigFileImperative DATA dragon.vtu suzanne.stl ARGS --edges CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json INTERACTION UI) #E;Right
  f3d_test(NAME TestInteractionConfigFileImperativeNoData CONFIG ${F3D_SOURCE_DIR}/testing/configs/imperative.json INTERACTION NO_DATA_FORCE_RENDER UI) #X;Up
  f3d_test(NAME TestInteractionConfigFileOptional DATA zombie.mdl f3d.glb CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #Right
endif()

## 2D Mode
f3d_test(NAME TestInteraction2DPan DATA cow.vtp ARGS --interaction-style=2d INTERACTION) #LeftMouse;MouseMovements
f3d_test(NAME TestInteraction2DZoom DATA cow.vtp ARGS --interaction-style=2d INTERACTION) #RightMouse;MouseMovements
f3d_test(NAME TestInteraction2DCycle DATA cow.vtp INTERACTION) #K;K;LeftMouse;MouseMovements

## Camera
f3d_test(NAME TestInteractionResetCamera DATA dragon.vtu INTERACTION LONG_TIMEOUT) #MouseMovements;Return;
f3d_test(NAME TestInteractionResetCameraWithCameraIndex DATA CameraAnimated.glb ARGS --camera-index=0 INTERACTION) #MouseMovements;Return;
f3d_test(NAME TestInteractionCameraUpdate DATA dragon.vtu INTERACTION) #MouseWheel;MouseWheel;MouseWheel;S
f3d_test(NAME TestInteractionFocalPointPickingDefault DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionFocalPointPickingShift DATA dragon.vtu INTERACTION LONG_TIMEOUT)
f3d_test(NAME TestInteractionFocalPointPickingPoints DATA pointsCloud.vtp INTERACTION THRESHOLD 0.05) # Threshold needed because sometime a point does not appear

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionDragRotateVertical DATA offset-flat-box.glb ARGS -g -x --up=y INTERACTION UI) # Drag down, should look straight down and avoid gimbal lock
endif()

f3d_test(NAME TestInteractionVerticalDragRotate DATA dragon.vtu ARGS --up=y INTERACTION LONG_TIMEOUT) # 7 (top-view); left click and drag
f3d_test(NAME TestInteractionInitUpVectorOption DATA dragon.vtu ARGS --up=-Y INTERACTION LONG_TIMEOUT) #Small drag left, camera should maintain orientation
f3d_test(NAME TestInteractionDynamicUpDirection DATA dragon.vtu INTERACTION) # Ctrl+Z
f3d_test(NAME TestInteractionCameraHotkeys DATA cow.vtp INTERACTION)
f3d_test(NAME TestInteractionZoomToMouse DATA cow.vtp INTERACTION)
f3d_test(NAME TestInteractionOrthographicProjection DATA cow.vtp INTERACTION) #5;5
f3d_test(NAME TestInteractionZoomToggleOrthographicProjection DATA cow.vtp INTERACTION) #MouseWheel;5;Mousewheelx6;5
f3d_test(NAME TestInteractionRotateCameraMinus90 DATA f3d.glb INTERACTION)
f3d_test(NAME TestInteractionRotateCamera90 DATA f3d.glb INTERACTION)
f3d_test(NAME TestInteractionRollCameraRotation DATA f3d.glb ARGS -g INTERACTION) #1;4;LeftMouse;MouseMovements
f3d_test(NAME TestInteractionElevationCameraRotation DATA f3d.glb ARGS -g INTERACTION THRESHOLD 0.06) #8;8;2 # Threshold is needed because camera seems to move slightly differently sometimes
f3d_test(NAME TestInteractionPanWithShift DATA f3d.glb INTERACTION) #Shift;LeftMouse;MouseMovements

# Test camera preserving/resetting when switching files in interaction mode
f3d_test(NAME TestInteractionSwitchFileNoCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)
f3d_test(NAME TestInteractionSwitchFilePrevCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)
f3d_test(NAME TestInteractionSwitchFileNextCameraKeeping DATA cow.vtp cowlow.vtp INTERACTION)

## Progress
f3d_test(NAME TestProgress DATA cow.vtp ARGS --loading-progress NO_BASELINE)
f3d_test(NAME TestProgressScene DATA WaterBottle.glb ARGS --loading-progress NO_BASELINE)
f3d_test(NAME TestInteractionProgressReload DATA cow.vtp ARGS --loading-progress NO_BASELINE INTERACTION) #Up;Up;Up;Up

## Animation
f3d_test(NAME TestInteractionAnimationNotStopped DATA InterpolationTest.glb NO_BASELINE INTERACTION) #Space;
f3d_test(NAME TestInteractionAnimationCycleAnimation DATA InterpolationTest.glb INTERACTION) #WWWWWWWWWWWW;Space;Space;
f3d_test(NAME TestInteractionAnimationIndexDeprecatedCycleAnimation DATA InterpolationTest.glb ARGS --animation-index=2 INTERACTION) #WWWWWWWWWWWW;Space;Space;
f3d_test(NAME TestInteractionCycleAnimationNoAnimation DATA cow.vtp INTERACTION NO_BASELINE) #W
f3d_test(NAME TestInteractionCycleAnimationOneAnimation DATA f3d.glb ARGS --verbose INTERACTION NO_BASELINE REGEXP "Current animation is: No animation") #W

if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250507)
  f3d_test(NAME TestInteractionAnimationCycleAnimationSingle DATA soldier_animations.mdl INTERACTION) #WWWWWWWWWWWW;Space;Space;
endif()

# Test interactive animation and speed factor
f3d_test(NAME TestInteractionAnimation DATA f3d.glb ARGS --animation-progress INTERACTION) #Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFast DATA f3d.glb ARGS --animation-progress --animation-speed-factor=1.5 INTERACTION) #Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationSlow DATA f3d.glb ARGS --animation-progress --animation-speed-factor=0.5 INTERACTION) #Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationFrameRate DATA f3d.glb ARGS --animation-progress --frame-rate=1 INTERACTION) #Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationCameraMovement DATA CameraAnimated.glb ARGS --camera-index=0 --animation-progress INTERACTION) #Space;MouseMovement;Space;
f3d_test(NAME TestInteractionAnimationInvert DATA f3d.glb ARGS --animation-speed-factor=-1 --animation-progress INTERACTION) #Space;Wait;Space;
f3d_test(NAME TestInteractionAnimationBackward DATA f3d.glb ARGS --animation-progress INTERACTION) #Ctrl+Shift+Space;Wait;Space;

## Cheatsheet
f3d_test(NAME TestInteractionCheatsheetWhiteBG DATA cow.vtp ARGS --background-color=1,1,1 INTERACTION UI) #H
f3d_test(NAME TestInteractionCheatsheetBlackBG DATA cow.vtp ARGS --background-color=0,0,0 INTERACTION UI) #H
f3d_test(NAME TestInteractionCheatsheetScalars DATA dragon.vtu ARGS --scalar-coloring --coloring-component=-2 INTERACTION UI LONG_TIMEOUT) #HSSS
f3d_test(NAME TestInteractionCheatsheetScalarsNoArray DATA dragon.vtu ARGS --scalar-coloring INTERACTION UI RESOLUTION 500,300) #H
f3d_test(NAME TestInteractionCheatsheetOpacity DATA cow.vtp INTERACTION UI ARGS --opacity=0.5 RESOLUTION 300,700) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetAnimationName DATA InterpolationTest.glb ARGS --animation-indices=6 INTERACTION UI) #HWWW
f3d_test(NAME TestInteractionCheatsheetConfigFile DATA dragon.vtu CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetMultiModifierBinding DATA dragon.vtu RESOLUTION 1200,300 CONFIG ${F3D_SOURCE_DIR}/testing/configs/bindings.json INTERACTION UI) #H;ScrollDown
f3d_test(NAME TestInteractionCheatsheetCycle DATA cow.vtp RESOLUTION 800,300 INTERACTION UI LONG_TIMEOUT) #HAAPO

# Need SSIM comparison for some reason
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240729)
  # Test scrolling cheatsheet with no model by mouse wheel and by using scrollbar
  f3d_test(NAME TestInteractionNoModelScrollWheel RESOLUTION 1000,600 NO_DATA_FORCE_RENDER LONG_TIMEOUT INTERACTION UI)
  f3d_test(NAME TestInteractionNoModelScrollBar RESOLUTION 1000,600 NO_DATA_FORCE_RENDER LONG_TIMEOUT INTERACTION UI)
endif()

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionCheatsheetNoFile INTERACTION UI NO_DATA_FORCE_RENDER) #HXM
endif()

if(F3D_MODULE_RAYTRACING)
  f3d_test(NAME TestInteractionCheatsheetCentered DATA cow.vtp RESOLUTION 500,1500 INTERACTION UI LONG_TIMEOUT) #H
endif()

## Scene Hierarchy
f3d_test(NAME TestInteractionSceneHierarchy DATA BoxAnimated.gltf INTERACTION UI) #Rotate;Shift+H;Click checkbox

## Console
f3d_test(NAME TestInteractionConsoleOpen DATA f3d.glb INTERACTION UI) #Escape
f3d_test(NAME TestInteractionConsoleOpenExit DATA f3d.glb REGEXP "Interactor has been stopped" INTERACTION NO_BASELINE UI) #Escape;exit;Return
f3d_test(NAME TestInteractionConsoleOpenWarningKeyboard DATA f3d.glb ARGS --position=0 INTERACTION UI) #Escape
f3d_test(NAME TestInteractionConsoleOpenWarningClick DATA f3d.glb ARGS --position=0 INTERACTION UI) #Click badge
f3d_test(NAME TestInteractionConsoleToggleGrid DATA f3d.glb INTERACTION UI) #Escape;toggle render.grid.enable;Return;Escape
f3d_test(NAME TestInteractionConsoleInvalidCommand DATA f3d.glb INTERACTION UI) #Escape;foo;Return
f3d_test(NAME TestInteractionConsoleInvalidOption DATA f3d.glb INTERACTION UI) #Escape;toggle foo;Return
f3d_test(NAME TestInteractionConsoleClear DATA dragon.vtu INTERACTION UI) #Escape;e;Escape;printt;BackSpace;_scee;Left;n;Right;_info;Return;clear;Return
f3d_test(NAME TestInteractionConsoleTypingSceneInfo RESOLUTION 400,300 DATA f3d.glb INTERACTION UI) #Escape;e;Escape;printt;BackSpace;_scee;Left;Right;_info;Return
f3d_test(NAME TestInteractionConsoleReload DATA f3d.glb INTERACTION UI) #Escape;reload_current_file_group;Return
f3d_test(NAME TestInteractionConsoleLoadNext DATA f3d.glb cow.vtp INTERACTION UI) #Escape;load_next_file_group;Return
f3d_test(NAME TestInteractionConsoleRender ARGS --coloring-by-cells DATA waveletMaterial.vti INTERACTION UI) #Escape;toggle_volume_rendering;Return
f3d_test(NAME TestInteractionConsoleCamera DATA f3d.glb INTERACTION UI) #Escape;set_camera top;Return
f3d_test(NAME TestInteractionConsoleScrollbar DATA f3d.glb INTERACTION UI) #Escape;a;Enter;Grab scrollbar
f3d_test(NAME TestInteractionConsoleEmptyCommand DATA f3d.glb INTERACTION UI) #Escape;Enter
f3d_test(NAME TestInteractionConsoleOverCheatSheet DATA f3d.glb INTERACTION UI) #h;Escape;Enter

# Need SSIM comparison for some reason
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240729)
  f3d_test(NAME TestInteractionConsoleAutoComplete DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab;Tab;_a;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteNoMatch DATA BoxAnimated.gltf INTERACTION UI) #Escape;zz;Tab
  f3d_test(NAME TestInteractionConsoleAutoCompleteCandidates DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab
  f3d_test(NAME TestInteractionConsoleAutoCompleteHide DATA BoxAnimated.gltf INTERACTION UI) #Escape;Tab;to;Tab;Esc;Esc;Esc
  f3d_test(NAME TestInteractionConsoleAutoCompleteToggleOptions DATA cow.vtp INTERACTION UI) #Escape;to;Tab;space;m;Tab;p;Tab;e;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteSetOptions ARGS --point-sprites=gaussian DATA cow.vtp INTERACTION UI RESOLUTION 400,300) #Escape;se;Tab;space;m;Tab;p;Tab;t;Tab;Tab;s;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteSetDisplayCompl DATA cow.vtp INTERACTION UI RESOLUTION 400,300) #Escape;se;Tab;space;m;Tab;p;Tab;t;Tab;Tab;Tab
  f3d_test(NAME TestInteractionConsoleAutoCompleteReaderOptions DATA BoxAnimated.gltf INTERACTION UI) #Escape;set_r;Tab;Q;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteColoring DATA BoxAnimated.gltf INTERACTION UI) #Escape;cy;Tab;;c;Tab;a;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteCamera DATA BoxAnimated.gltf INTERACTION UI) #Escape;se;Tab;_c;Tab;i;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteHelp DATA BoxAnimated.gltf INTERACTION UI) #Escape;h;Tab;h;Tab;Enter
  f3d_test(NAME TestInteractionConsoleAutoCompleteTokenizeError DATA BoxAnimated.gltf INTERACTION UI NO_BASELINE REGEXP "unable to tokenize") #Escape;";Tab;

  f3d_test(NAME TestInteractionConsoleHistory DATA BoxAnimated.gltf INTERACTION UI) #Escape;ro;Tab;45;Enter;set_c;Tab;top;Enter;Up;Up;Enter;Up;Up;Up;Up;Down;Down;Down;Down
  f3d_test(NAME TestInteractionConsoleHistoryEmpty DATA BoxAnimated.gltf INTERACTION UI) #Escape;Up;Down
  f3d_test(NAME TestInteractionConsoleHistoryHide DATA BoxAnimated.gltf INTERACTION UI) #Escape;a;Enter;b;Enter;Up;Escape;Escape;Escape;Up

  file(COPY "${F3D_SOURCE_DIR}/testing/data/cow.vtp" DESTINATION "${CMAKE_BINARY_DIR}/Testing/data/") # Capital T, to avoid confusion on Windows and in-sources build
  # First file (`.d`) doesn't exist and is there to test multi args
  f3d_test(NAME TestInteractionConsoleAutoCompleteFilesystem INTERACTION UI) #Escape;ad;Tab;_f;Tab; ;.;Tab;d ..;Tab;T;Tab;i;Tab;d;Tab;Tab;Enter;Esc
  if(WIN32 OR APPLE)
    f3d_test(NAME TestInteractionConsoleAutoCompleteDirCaseInsensitiveUppercaseF INTERACTION UI NO_DATA_FORCE_RENDER WORKING_DIR ${F3D_SOURCE_DIR}/testing/data/AutoCompletion) #Escape;add_files F;Tab
    f3d_test(NAME TestInteractionConsoleAutoCompleteDirCaseInsensitiveLowercaseF INTERACTION UI NO_DATA_FORCE_RENDER WORKING_DIR ${F3D_SOURCE_DIR}/testing/data/AutoCompletion) #Escape;add_files f;Tab
  else()
    f3d_test(NAME TestInteractionConsoleAutoCompleteDirCaseSensitive INTERACTION UI NO_DATA_FORCE_RENDER WORKING_DIR ${F3D_SOURCE_DIR}/testing/data/AutoCompletion) #Escape;add_files F;Tab
  endif()

  if(NOT WIN32)
    # Does not work on windows as it uses hard coded slash
    f3d_test(NAME TestInteractionConsoleAutoCompleteFilesystemError INTERACTION UI NO_BASELINE LONG_TIMEOUT REGEXP "File name too long") #Escape;ad;Tab;_f;Tab; ;Tab;300*a/a;Tab;Esc
  endif()
endif()

f3d_test(NAME TestInteractionMinimalConsoleOpen DATA f3d.glb INTERACTION UI) #Colon
f3d_test(NAME TestInteractionMinimalConsoleEscape DATA f3d.glb INTERACTION UI) #Colon;Escape
f3d_test(NAME TestInteractionMinimalConsoleReturn DATA f3d.glb INTERACTION UI) #Colon;Return
f3d_test(NAME TestInteractionMinimalConsoleValidCommand DATA f3d.glb INTERACTION UI) #Colon;set_camera top;Return
f3d_test(NAME TestInteractionMinimalConsoleInvalidCommand DATA f3d.glb INTERACTION UI) #Colon;foo;Return
f3d_test(NAME TestInteractionMinimalConsoleResize DATA f3d.glb INTERACTION UI) #Colon;foo;Return;Colon
f3d_test(NAME TestInteractionMinimalConsoleBadgeRemains DATA f3d.glb INTERACTION UI) #Colon;foo;Return;Colon;Escape
f3d_test(NAME TestInteractionMinimalConsoleOverCheatSheet DATA f3d.glb INTERACTION UI) #h;:
f3d_test(NAME TestInteractionMinimalConsoleOverCheatSheetAndFilename DATA f3d.glb INTERACTION UI) #h;n;:

## HDRI
f3d_test(NAME TestInteractionHDRIMove DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION THRESHOLD 0.05) #Shift+MouseRight; # Threshold is needed for legacy image comparison for VTK 9.3
f3d_test(NAME TestInteractionHDRIBlur DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #U
f3d_test(NAME TestInteractionHDRIReload DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION) #Up

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionHDRIChange DATA multi HDRI palermo_park_1k.hdr CONFIG ${F3D_SOURCE_DIR}/testing/configs/complex.json INTERACTION UI) #Left
endif()

f3d_test(NAME TestInteractionHDRICache DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION DEPENDS TestHDRI) #FFFFJJJJ
f3d_test(NAME TestInteractionHDRIRemoveSkybox DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIRemoveAmbient DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIRemoveBoth DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRILoop DATA suzanne.ply HDRI palermo_park_1k.hdr INTERACTION)
f3d_test(NAME TestInteractionHDRIFullFromNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr LONG_TIMEOUT INTERACTION)

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  # Interaction HDRI tests
  f3d_test(NAME TestInteractionDropHDRI INTERACTION_CONFIGURE LONG_TIMEOUT UI) #X;DropEvent dragon.vtu;DropEvent palermo.hdr;
  f3d_test(NAME TestInteractionDropHDRIInvert INTERACTION_CONFIGURE LONG_TIMEOUT UI) #X;DropEvent palermo.hdr;DropEvent dragon.vtu;
  f3d_test(NAME TestInteractionDropHDRIMulti INTERACTION_CONFIGURE LONG_TIMEOUT UI) #X;DropEvent dragon.vtu palermo.hdr;
endif()

if(F3D_MODULE_RAYTRACING)
  # https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10753
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231213)
    # XXX: These tests are impacted by https://github.com/f3d-app/f3d/issues/933
    f3d_test(NAME TestInteractionHDRIRaytracingRemoveSkybox DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingRemoveAmbient DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingRemoveBoth DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingLoop DATA suzanne.ply HDRI palermo_park_1k.hdr ARGS -rd --raytracing-samples=4 INTERACTION)
    f3d_test(NAME TestInteractionHDRIRaytracingFullFromNone DATA suzanne.ply ARGS --hdri-file=${F3D_SOURCE_DIR}/testing/data/palermo_park_1k.hdr -rd --raytracing-samples=4 INTERACTION LONG_TIMEOUT)
  endif()
endif()

## Drops
f3d_test(NAME TestInteractionEmptyDrop INTERACTION REGEXP "Drop event without any provided files.") #DropEvent Empty;

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
  f3d_test(NAME TestInteractionDropFiles ARGS -n INTERACTION_CONFIGURE UI) #X;DropEvent cow.vtp;DropEvent dragon.vtu suzanne.stl;
  f3d_test(NAME TestInteractionDropSameFiles ARGS -x INTERACTION_CONFIGURE UI) #DropEvent cow.vtp;#DropEvent dragon.vtu;#DropEvent cow.vtp#DropEvent cow.vtp;
endif()
f3d_test(NAME TestInteractionMultiFileDrop ARGS --multi-file-mode=all -e INTERACTION_CONFIGURE) #DropEvent mb_1_0.vtp mb_2_0.vtp

# A proper test for this is not possible because of the double quotes
f3d_test(NAME TestInteractionDropFileWithQuotes ARGS -n INTERACTION REGEXP "\"'`Quotes\"'`.stl does not exist" NO_BASELINE) #X;DropEvent "'`Quotes"'`.stl";

if(WIN32)
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    # Windows specific drop test, using backslashes
    f3d_test(NAME TestInteractionDropFileBackslashes ARGS -n INTERACTION_CONFIGURE) #X;DropEvent path\to\cow.vtp;
  endif()
endif()

# Test modified drops, this test rendering is impacted by https://github.com/f3d-app/f3d/issues/1558
# Empty drop is for coverage
f3d_test(NAME TestInteractionDropHDRIModifiers INTERACTION_CONFIGURE LONG_TIMEOUT) #CTRL+DropEvent f3d.tif;SHIFT+DropEvent;SHIFT+DropEvent palermo.tif;SYYYY

# Test interactive animation and dropping HDRI
f3d_test(NAME TestInteractionAnimationDropHDRI DATA InterpolationTest.glb ARGS --animation-indices=-1 --animation-progress INTERACTION_CONFIGURE LONG_TIMEOUT) #Space;DropEvent palermo.hdr;Space;

if(F3D_MODULE_EXR)
  # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
  if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
    f3d_test(NAME TestInteractionDropHDRIExr INTERACTION_CONFIGURE LONG_TIMEOUT) #X;DropEvent kloofendal.exr;DropEvent dragon.vtu;
  endif()
endif()
