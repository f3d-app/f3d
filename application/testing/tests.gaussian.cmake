## Gaussian Splatting tests, using only native plugin

# Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10662
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20231102)
  f3d_test(NAME Test3DGaussiansSplattingStochastic DATA small.splat ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=stochastic --camera-position=-3.6,0.5,-4.2)
endif()

# Needs splat sorting with compute shaders
if(VTK_VERSION VERSION_GREATER_EQUAL 9.3.20240203)
  if(NOT APPLE) # MacOS does not support compute shaders
    f3d_test(NAME Test3DGaussiansSplatting DATA small.splat ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,-4.2)
    # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
      f3d_test(NAME TestDefaultConfigFileSPLAT DATA small.splat CONFIG config_build LONG_TIMEOUT UI)
    endif()
    f3d_test(NAME TestThumbnailConfigFileSPLAT DATA small.splat CONFIG thumbnail_build LONG_TIMEOUT)

    f3d_test(NAME Test3DGSPLY DATA bonsai_small.ply ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
    f3d_test(NAME Test3DGSPLYHDRI DATA bonsai_small.ply HDRI shanghai_bund_1k.hdr ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
    f3d_test(NAME TestInteractionPointSpritesCycle DATA bonsai_small.ply ARGS -sy --point-sprites-absolute-size --up=-Y --blending=sort --camera-position=-2.6,0.5,-3.2 INTERACTION) #OOOOO

    f3d_test(NAME TestSPZDegree0 DATA hornedlizard_small_d0.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
    f3d_test(NAME TestInvalidSH DATA invalidSH.vtp ARGS -osy --verbose REGEXP "Spherical harmonics array is not valid" NO_BASELINE)

    # Needs texture array support: https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12112
    if(VTK_VERSION VERSION_GREATER_EQUAL 9.4.20250513)
      f3d_test(NAME TestSPZDegree1 DATA hornedlizard_small_d1.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree2 DATA hornedlizard_small_d2.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree3 DATA hornedlizard_small_d3.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-3.6,0.5,4.2)
      f3d_test(NAME TestSPZDegree1Stochastic DATA hornedlizard_small_d1.spz ARGS -sy --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=stochastic --camera-position=-3.6,0.5,4.2) # Test instancing with spherical harmonics

      # Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/12489
      if(VTK_VERSION VERSION_GREATER_EQUAL 9.5.20251001)
        f3d_test(NAME TestDefaultConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG config_build LONG_TIMEOUT UI)
      endif()
      f3d_test(NAME TestThumbnailConfigFileSPZ DATA hornedlizard_small_d3.spz CONFIG thumbnail_build LONG_TIMEOUT)
      f3d_test(NAME TestSPZVersion3 DATA bonsai.spz ARGS -sy --up=-Y --point-sprites-absolute-size --point-sprites-size=1 --point-sprites=gaussian --blending=sort --camera-position=-2.6,0.5,-3.2)
      f3d_test(NAME TestInvalidSPZTruncated DATA invalid_spz_gzip_truncated.spz ARGS -osy --verbose REGEXP "Invalid GZIP file" NO_BASELINE)
      f3d_test(NAME TestInvalidSPZMagic DATA invalid_spz_magic.spz ARGS -osy --verbose REGEXP "Incompatible SPZ header" NO_BASELINE)
      f3d_test(NAME TestInvalidSPZVersion DATA invalid_spz_version.spz ARGS -osy --verbose REGEXP "Incompatible SPZ version. Only 2 and 3 are supported" NO_BASELINE)
    endif()
  endif()
endif()
