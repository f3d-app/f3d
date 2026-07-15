// Clip Library
// Copyright (c) 2015-2026 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
// 
// This file was originally copied from VTK, under BSD-3-Clause license
// Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen

#include "clip_xcb_functions.h"

#include <string>
#include <dlfcn.h>

#define NULLIFY_POINTER_TO_FUNCTION(name) name = nullptr
#define DEFINE_POINTER_TO_FUNCTION(name) name##_type NULLIFY_POINTER_TO_FUNCTION(name)
#define LOAD_POINTER_TO_FUNCTION(lib, symbol, name)           \
name = reinterpret_cast<name##_type>(dlsym(lib, #symbol));    \
if (name == nullptr)                                          \
{                                                             \
  throw xcb_load_failure_exception(std::string("Symbol ") + #symbol + " is missing from libxcb"); \
}

static void* libxcb = nullptr;
DEFINE_POINTER_TO_FUNCTION(clip_xcb_connect);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_setup);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_setup_roots_iterator);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_generate_id);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_create_window);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_destroy_window);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_flush);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_disconnect);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_send_event);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_wait_for_event);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_change_property);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_property_value);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_property_value_length);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_property);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_property_reply);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_convert_selection);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_intern_atom);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_intern_atom_reply);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_set_selection_owner_checked);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_selection_owner);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_get_selection_owner_reply);
DEFINE_POINTER_TO_FUNCTION(clip_xcb_request_check);

void clip_xcb_functions_initialize()
{
  for (const char* libName : { "libxcb.so.1.1.0", "libxcb.so.1", "libxcb.so" })
  {
    libxcb = dlopen(libName, RTLD_LAZY | RTLD_LOCAL);
    if (libxcb != nullptr)
    {
      break;
    }
  }
  if (libxcb == nullptr)
  {
    throw xcb_load_failure_exception("Could not find libxcb");
  }
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_connect, clip_xcb_connect);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_setup, clip_xcb_get_setup);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_setup_roots_iterator, clip_xcb_setup_roots_iterator);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_generate_id, clip_xcb_generate_id);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_create_window, clip_xcb_create_window);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_destroy_window, clip_xcb_destroy_window);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_flush, clip_xcb_flush);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_disconnect, clip_xcb_disconnect);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_send_event, clip_xcb_send_event);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_wait_for_event, clip_xcb_wait_for_event);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_change_property, clip_xcb_change_property);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_property_value, clip_xcb_get_property_value);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_property_value_length, clip_xcb_get_property_value_length);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_property, clip_xcb_get_property);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_property_reply, clip_xcb_get_property_reply);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_convert_selection, clip_xcb_convert_selection);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_intern_atom, clip_xcb_intern_atom);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_intern_atom_reply, clip_xcb_intern_atom_reply);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_set_selection_owner_checked, clip_xcb_set_selection_owner_checked);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_selection_owner, clip_xcb_get_selection_owner);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_get_selection_owner_reply, clip_xcb_get_selection_owner_reply);
  LOAD_POINTER_TO_FUNCTION(libxcb, xcb_request_check, clip_xcb_request_check);
}

void clip_xcb_functions_finalize()
{
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_connect);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_setup);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_setup_roots_iterator);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_generate_id);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_create_window);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_destroy_window);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_flush);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_disconnect);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_send_event);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_wait_for_event);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_change_property);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_property_value);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_property_value_length);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_property);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_property_reply);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_convert_selection);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_intern_atom);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_intern_atom_reply);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_set_selection_owner_checked);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_selection_owner);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_get_selection_owner_reply);
  NULLIFY_POINTER_TO_FUNCTION(clip_xcb_request_check);

  if (libxcb)
  {
    dlclose(libxcb);
    libxcb = nullptr;
  }
}

xcb_load_failure_exception::xcb_load_failure_exception(const std::string& what)
  : std::runtime_error(what)
{
}
