# Clip Library
# Copyright (C) 2015-2024 David Capello

function(add_example name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} clip)
  target_include_directories(${name} PUBLIC ..)
endfunction()

add_example(copy)
add_example(hello_world)
add_example(int_format)
add_example(paste)
if(CLIP_ENABLE_IMAGE)
  add_example(put_image)
  add_example(show_image)
endif()
if(CLIP_ENABLE_LIST_FORMATS)
  add_example(list_clip_formats)
endif()
