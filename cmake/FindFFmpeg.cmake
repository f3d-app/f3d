#[==[
Provides the following variables:

  * `FFmpeg_INCLUDE_DIR`: Include directories necessary to use FFMPEG.
  * `FFmpeg_LIBRARIES`: Libraries necessary to use FFMPEG.
  * `FFmpeg_AVCODEC_LIBRARY`: The avcodec library.
  * `FFmpeg_AVUTIL_LIBRARY`: The avutil library.

It also provides the following targets:

  * `FFmpeg::avcodec`: A target to use with `target_link_libraries`.
  * `FFmpeg::avutil`: A target to use with `target_link_libraries`.

Note that this module only checks for the presence of the `avcodec` and `avutil` libraries, which are required to use FFMPEG.
This file is required because FFmpeg is pkg-config based and does not provide a CMake config file.
#]==]

include(FindPackageHandleStandardArgs)

find_path(FFmpeg_INCLUDE_DIR
  NAMES libavcodec/avcodec.h
)

find_library(FFmpeg_AVCODEC_LIBRARY
  NAMES avcodec libavcodec
)

find_library(FFmpeg_AVUTIL_LIBRARY
  NAMES avutil libavutil
)

find_package_handle_standard_args(FFmpeg
  REQUIRED_VARS
    FFmpeg_INCLUDE_DIR
    FFmpeg_AVCODEC_LIBRARY
    FFmpeg_AVUTIL_LIBRARY
)

if(FFmpeg_FOUND)
  set(FFmpeg_INCLUDE_DIRS ${FFmpeg_INCLUDE_DIR})

  set(FFmpeg_LIBRARIES
    ${FFmpeg_AVCODEC_LIBRARY}
    ${FFmpeg_AVUTIL_LIBRARY}
  )

  if(NOT TARGET FFmpeg::avutil)
    add_library(FFmpeg::avutil UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avutil PROPERTIES
      IMPORTED_LOCATION "${FFmpeg_AVUTIL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIR}"
    )
  endif()

  if(NOT TARGET FFmpeg::avcodec)
    add_library(FFmpeg::avcodec UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avcodec PROPERTIES
      IMPORTED_LOCATION "${FFmpeg_AVCODEC_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES FFmpeg::avutil
    )
  endif()
endif()

mark_as_advanced(
  FFmpeg_INCLUDE_DIR
  FFmpeg_AVCODEC_LIBRARY
  FFmpeg_AVUTIL_LIBRARY
)
