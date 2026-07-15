// Clip Library
// Copyright (c) 2015-2026 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
//
// This file was originally copied from VTK, under BSD-3-Clause license
// Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
//
// Dynamically load libxcb and provides function pointers to the API clip needs
// in clip_x11.cpp

#ifndef CLIB_XCB_FUNCTIONS_H_INCLUDED
#define CLIB_XCB_FUNCTIONS_H_INCLUDED

#define CLIP_XCB_EXPORT __attribute__((visibility("default")))

#include <xcb/xcb.h>
#include <stdexcept>
#include <string>

typedef xcb_connection_t* (*clip_xcb_connect_type)(const char *, int *);
typedef const struct xcb_setup_t* (*clip_xcb_get_setup_type)(xcb_connection_t *);
typedef xcb_screen_iterator_t 	(*clip_xcb_setup_roots_iterator_type) (const xcb_setup_t*);
typedef uint32_t (*clip_xcb_generate_id_type)(xcb_connection_t*);
typedef xcb_void_cookie_t (*clip_xcb_create_window_type)(xcb_connection_t*, uint8_t, xcb_window_t, xcb_window_t, int16_t,	int16_t, uint16_t, uint16_t, uint16_t ,	uint16_t, xcb_visualid_t,	uint32_t, const void*);
typedef xcb_void_cookie_t (*clip_xcb_destroy_window_type)(xcb_connection_t*, xcb_window_t);
typedef int (*clip_xcb_flush_type)(xcb_connection_t*);
typedef void (*clip_xcb_disconnect_type)(xcb_connection_t*);
typedef xcb_void_cookie_t (*clip_xcb_send_event_type)(xcb_connection_t*, uint8_t,	xcb_window_t,	uint32_t, const char*);
typedef xcb_generic_event_t* (*clip_xcb_wait_for_event_type)(xcb_connection_t*);
typedef xcb_void_cookie_t (*clip_xcb_change_property_type)(xcb_connection_t*, uint8_t, xcb_window_t, xcb_atom_t, xcb_atom_t, uint8_t, uint32_t, const void*);
typedef void* (*clip_xcb_get_property_value_type)(const xcb_get_property_reply_t*);
typedef int (*clip_xcb_get_property_value_length_type)(const xcb_get_property_reply_t*);
typedef xcb_get_property_cookie_t (*clip_xcb_get_property_type)(xcb_connection_t*, uint8_t, xcb_window_t, xcb_atom_t, xcb_atom_t, uint32_t, uint32_t);
typedef xcb_get_property_reply_t* (*clip_xcb_get_property_reply_type)(xcb_connection_t*, xcb_get_property_cookie_t, xcb_generic_error_t**);
typedef xcb_void_cookie_t (*clip_xcb_convert_selection_type)(xcb_connection_t*, xcb_window_t, xcb_atom_t, xcb_atom_t, xcb_atom_t, xcb_timestamp_t);
typedef xcb_intern_atom_cookie_t (*clip_xcb_intern_atom_type)(xcb_connection_t*, uint8_t, uint16_t,	const char*);
typedef xcb_intern_atom_reply_t* (*clip_xcb_intern_atom_reply_type)(xcb_connection_t*, xcb_intern_atom_cookie_t, xcb_generic_error_t**);
typedef xcb_void_cookie_t (*clip_xcb_set_selection_owner_checked_type)(xcb_connection_t*, xcb_window_t, xcb_atom_t, xcb_timestamp_t);
typedef xcb_get_selection_owner_cookie_t (*clip_xcb_get_selection_owner_type)(xcb_connection_t*, xcb_atom_t);
typedef xcb_get_selection_owner_reply_t* (*clip_xcb_get_selection_owner_reply_type)(xcb_connection_t*, xcb_get_selection_owner_cookie_t, xcb_generic_error_t**);
typedef xcb_generic_error_t* (*clip_xcb_request_check_type)(xcb_connection_t*, xcb_void_cookie_t);

CLIP_XCB_EXPORT extern clip_xcb_connect_type clip_xcb_connect;
CLIP_XCB_EXPORT extern clip_xcb_get_setup_type clip_xcb_get_setup;
CLIP_XCB_EXPORT extern clip_xcb_setup_roots_iterator_type clip_xcb_setup_roots_iterator;
CLIP_XCB_EXPORT extern clip_xcb_generate_id_type clip_xcb_generate_id;
CLIP_XCB_EXPORT extern clip_xcb_create_window_type clip_xcb_create_window;
CLIP_XCB_EXPORT extern clip_xcb_destroy_window_type clip_xcb_destroy_window;
CLIP_XCB_EXPORT extern clip_xcb_flush_type clip_xcb_flush;
CLIP_XCB_EXPORT extern clip_xcb_disconnect_type clip_xcb_disconnect;
CLIP_XCB_EXPORT extern clip_xcb_send_event_type clip_xcb_send_event;
CLIP_XCB_EXPORT extern clip_xcb_wait_for_event_type clip_xcb_wait_for_event;
CLIP_XCB_EXPORT extern clip_xcb_change_property_type clip_xcb_change_property;
CLIP_XCB_EXPORT extern clip_xcb_get_property_value_type clip_xcb_get_property_value;
CLIP_XCB_EXPORT extern clip_xcb_get_property_value_length_type clip_xcb_get_property_value_length;
CLIP_XCB_EXPORT extern clip_xcb_get_property_type clip_xcb_get_property;
CLIP_XCB_EXPORT extern clip_xcb_get_property_reply_type clip_xcb_get_property_reply;
CLIP_XCB_EXPORT extern clip_xcb_convert_selection_type clip_xcb_convert_selection;
CLIP_XCB_EXPORT extern clip_xcb_intern_atom_type clip_xcb_intern_atom;
CLIP_XCB_EXPORT extern clip_xcb_intern_atom_reply_type clip_xcb_intern_atom_reply;
CLIP_XCB_EXPORT extern clip_xcb_set_selection_owner_checked_type clip_xcb_set_selection_owner_checked;
CLIP_XCB_EXPORT extern clip_xcb_get_selection_owner_type clip_xcb_get_selection_owner;
CLIP_XCB_EXPORT extern clip_xcb_get_selection_owner_reply_type clip_xcb_get_selection_owner_reply;
CLIP_XCB_EXPORT extern clip_xcb_request_check_type clip_xcb_request_check;

/**
 * Initialize the xcb function pointers by dynamically loading them from libxcb.so.
 * Must be called before using any of the function pointers.
 * Safe to call multiple times; subsequent calls have no effect.
 */
CLIP_XCB_EXPORT void clip_xcb_functions_initialize();

/**
 * Finalize the xcb function pointers, releasing any resources.
 * Should be called when done using the function pointers.
 * Safe to call multiple times; subsequent calls have no effect.
 */
CLIP_XCB_EXPORT void clip_xcb_functions_finalize();

/**
 * An exception that can be thrown by the xcb loader on failure
 */
struct xcb_load_failure_exception : public std::runtime_error
{
  explicit xcb_load_failure_exception(const std::string& what = "");
};

#endif // CLIB_XCB_FUNCTIONS_H_INCLUDED
