#ifndef F3D_INTERACTOR_C_API_H
#define F3D_INTERACTOR_C_API_H

#include "export.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Opaque handle to an f3d::interactor object.
   */
  typedef struct f3d_interactor_t f3d_interactor_t;

  /**
   * @brief Enumeration of supported modifier key combinations.
   */
  typedef enum f3d_interaction_bind_modifier_keys_t
  {
    F3D_INTERACTION_BIND_ANY = 0x80,
    F3D_INTERACTION_BIND_NONE = 0x0,
    F3D_INTERACTION_BIND_CTRL = 0x1,
    F3D_INTERACTION_BIND_SHIFT = 0x2,
    F3D_INTERACTION_BIND_CTRL_SHIFT = 0x3
  } f3d_interaction_bind_modifier_keys_t;

  /**
   * @brief Structure representing an interaction binding.
   */
  typedef struct f3d_interaction_bind_t
  {
    f3d_interaction_bind_modifier_keys_t mod;
    char inter[256]; // String for the interaction
  } f3d_interaction_bind_t;

  /**
   * @brief Format an interaction bind into a string.
   *
   * Formats the bind into a string like "A", "Any+Question", "Shift+L", etc.
   * The output buffer must be at least 512 bytes.
   *
   * @param bind Interaction bind to format.
   * @param output Output buffer to store the formatted string.
   * @param output_size Size of the output buffer.
   */
  F3D_EXPORT void f3d_interaction_bind_format(
    const f3d_interaction_bind_t* bind, char* output, int output_size);

  /**
   * @brief Parse a string into an interaction bind.
   *
   * Creates an interaction bind from a string like "A", "Ctrl+A", "Shift+B", etc.
   *
   * @param str String to parse.
   * @param bind Output parameter for the parsed bind.
   */
  F3D_EXPORT void f3d_interaction_bind_parse(const char* str, f3d_interaction_bind_t* bind);

  /**
   * @brief Compare two interaction binds for less-than ordering.
   *
   * Compares modifier and interaction string for ordering.
   * Useful for storing binds in sorted data structures.
   *
   * @param lhs Left-hand side bind.
   * @param rhs Right-hand side bind.
   * @return 1 if lhs < rhs, 0 otherwise.
   */
  F3D_EXPORT int f3d_interaction_bind_less_than(
    const f3d_interaction_bind_t* lhs, const f3d_interaction_bind_t* rhs);

  /**
   * @brief Compare two interaction binds for equality.
   *
   * Compares both modifier and interaction string for equality.
   *
   * @param lhs Left-hand side bind.
   * @param rhs Right-hand side bind.
   * @return 1 if binds are equal, 0 otherwise.
   */
  F3D_EXPORT int f3d_interaction_bind_equals(
    const f3d_interaction_bind_t* lhs, const f3d_interaction_bind_t* rhs);

  /**
   * @brief Enumeration of binding types.
   */
  typedef enum f3d_interactor_binding_type_t
  {
    F3D_INTERACTOR_BINDING_CYCLIC = 0,
    F3D_INTERACTOR_BINDING_NUMERICAL = 1,
    F3D_INTERACTOR_BINDING_TOGGLE = 2,
    F3D_INTERACTOR_BINDING_OTHER = 3
  } f3d_interactor_binding_type_t;

  /**
   * @brief Enumeration of supported mouse buttons.
   */
  typedef enum f3d_interactor_mouse_button_t
  {
    F3D_INTERACTOR_MOUSE_LEFT,
    F3D_INTERACTOR_MOUSE_RIGHT,
    F3D_INTERACTOR_MOUSE_MIDDLE
  } f3d_interactor_mouse_button_t;

  /**
   * @brief Enumeration of supported mouse wheel directions.
   */
  typedef enum f3d_interactor_wheel_direction_t
  {
    F3D_INTERACTOR_WHEEL_FORWARD,
    F3D_INTERACTOR_WHEEL_BACKWARD,
    F3D_INTERACTOR_WHEEL_LEFT,
    F3D_INTERACTOR_WHEEL_RIGHT
  } f3d_interactor_wheel_direction_t;

  /**
   * @brief Enumeration of supported input actions.
   */
  typedef enum f3d_interactor_input_action_t
  {
    F3D_INTERACTOR_INPUT_PRESS,
    F3D_INTERACTOR_INPUT_RELEASE
  } f3d_interactor_input_action_t;

  /**
   * @brief Enumeration of supported input modifiers.
   */
  typedef enum f3d_interactor_input_modifier_t
  {
    F3D_INTERACTOR_INPUT_NONE,
    F3D_INTERACTOR_INPUT_CTRL,
    F3D_INTERACTOR_INPUT_SHIFT,
    F3D_INTERACTOR_INPUT_CTRL_SHIFT
  } f3d_interactor_input_modifier_t;

  /**
   * @brief Enumeration of animation direction.
   */
  typedef enum f3d_interactor_animation_direction_t
  {
    F3D_INTERACTOR_ANIMATION_FORWARD,
    F3D_INTERACTOR_ANIMATION_BACKWARD
  } f3d_interactor_animation_direction_t;

  ///@{ @name Commands
  /**
   * @brief Initialize commands (remove existing and add defaults).
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_init_commands(f3d_interactor_t* interactor);

  /**
   * @brief Callback signature for command execution.
   */
  typedef void (*f3d_interactor_command_callback_t)(
    const char** args, int arg_count, void* user_data);

  /**
   * @brief Add a command with the provided action.
   *
   * @param interactor Interactor handle.
   * @param action Action string.
   * @param callback Command callback function.
   * @param user_data Optional user data passed to callback.
   */
  F3D_EXPORT void f3d_interactor_add_command(f3d_interactor_t* interactor, const char* action,
    f3d_interactor_command_callback_t callback, void* user_data);

  /**
   * @brief Remove a command for the provided action.
   *
   * @param interactor Interactor handle.
   * @param action Action string.
   */
  F3D_EXPORT void f3d_interactor_remove_command(f3d_interactor_t* interactor, const char* action);

  /**
   * @brief Get all command actions.
   *
   * @param interactor Interactor handle.
   * @param count Output parameter for number of actions.
   * @return Array of action strings. Caller must free the array with
   *         f3d_interactor_free_string_array().
   */
  F3D_EXPORT char** f3d_interactor_get_command_actions(f3d_interactor_t* interactor, int* count);

  /**
   * @brief Trigger a command.
   *
   * @param interactor Interactor handle.
   * @param command Command string.
   * @param keep_comments If non-zero, comments with # are supported.
   * @return 1 if command succeeded, 0 otherwise.
   */
  F3D_EXPORT int f3d_interactor_trigger_command(
    f3d_interactor_t* interactor, const char* command, int keep_comments);
  ///@}

  ///@{ @name Bindings
  /**
   * @brief Initialize bindings (remove existing and add defaults).
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_init_bindings(f3d_interactor_t* interactor);

  /**
   * @brief Add a binding for the provided bind.
   *
   * @param interactor Interactor handle.
   * @param bind Interaction bind.
   * @param commands Array of command strings.
   * @param command_count Number of commands.
   * @param group Optional group name (can be NULL).
   */
  F3D_EXPORT void f3d_interactor_add_binding(f3d_interactor_t* interactor,
    const f3d_interaction_bind_t* bind, const char** commands, int command_count,
    const char* group);

  /**
   * @brief Remove a binding for the provided bind.
   *
   * @param interactor Interactor handle.
   * @param bind Interaction bind.
   */
  F3D_EXPORT void f3d_interactor_remove_binding(
    f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind);

  /**
   * @brief Get all bind groups.
   *
   * @param interactor Interactor handle.
   * @param count Output parameter for number of groups.
   * @return Array of group strings. Caller must free the array with
   *         f3d_interactor_free_string_array().
   */
  F3D_EXPORT char** f3d_interactor_get_bind_groups(f3d_interactor_t* interactor, int* count);

  /**
   * @brief Get all binds for a specific group.
   *
   * @param interactor Interactor handle.
   * @param group Group name.
   * @param count Output parameter for number of binds.
   * @return Array of binds. Caller must free the array with
   *         f3d_interactor_free_bind_array().
   */
  F3D_EXPORT f3d_interaction_bind_t* f3d_interactor_get_binds_for_group(
    f3d_interactor_t* interactor, const char* group, int* count);

  /**
   * @brief Get all binds.
   *
   * @param interactor Interactor handle.
   * @param count Output parameter for number of binds.
   * @return Array of binds. Caller must free the array with
   *         f3d_interactor_free_bind_array().
   */
  F3D_EXPORT f3d_interaction_bind_t* f3d_interactor_get_binds(
    f3d_interactor_t* interactor, int* count);

  /**
   * @brief Structure containing binding documentation.
   */
  typedef struct f3d_binding_documentation_t
  {
    char doc[512];   // Documentation string
    char value[256]; // Current value string
  } f3d_binding_documentation_t;

  /**
   * @brief Get documentation for a binding.
   *
   * @param interactor Interactor handle.
   * @param bind Interaction bind.
   * @param doc Output parameter for documentation.
   */
  F3D_EXPORT void f3d_interactor_get_binding_documentation(f3d_interactor_t* interactor,
    const f3d_interaction_bind_t* bind, f3d_binding_documentation_t* doc);

  /**
   * @brief Get the type of a binding.
   *
   * @param interactor Interactor handle.
   * @param bind Interaction bind.
   * @return Binding type.
   */
  F3D_EXPORT f3d_interactor_binding_type_t f3d_interactor_get_binding_type(
    f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind);
  ///@}

  ///@{ @name Animation
  /**
   * @brief Toggle the animation.
   *
   * @param interactor Interactor handle.
   * @param direction Animation direction.
   */
  F3D_EXPORT void f3d_interactor_toggle_animation(
    f3d_interactor_t* interactor, f3d_interactor_animation_direction_t direction);

  /**
   * @brief Start the animation.
   *
   * @param interactor Interactor handle.
   * @param direction Animation direction.
   */
  F3D_EXPORT void f3d_interactor_start_animation(
    f3d_interactor_t* interactor, f3d_interactor_animation_direction_t direction);

  /**
   * @brief Stop the animation.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_stop_animation(f3d_interactor_t* interactor);

  /**
   * @brief Check if animation is currently playing.
   *
   * @param interactor Interactor handle.
   * @return 1 if animation is playing, 0 otherwise.
   */
  F3D_EXPORT int f3d_interactor_is_playing_animation(f3d_interactor_t* interactor);

  /**
   * @brief Get the current animation direction.
   *
   * @param interactor Interactor handle.
   * @return Current animation direction.
   */
  F3D_EXPORT f3d_interactor_animation_direction_t f3d_interactor_get_animation_direction(
    f3d_interactor_t* interactor);
  ///@}

  /**
   * @brief Load animation at provided frame value
   * When relative is false frame -1 is equal to last frame
   *
   * @param interactor Interactor handle.
   * @param frame Frame to load.
   * @param relative Use current frame as jump computation starting point.
   */
  F3D_EXPORT void f3d_interactor_jump_to_frame(
    f3d_interactor_t* interactor, int frame, bool relative);
  ///@}

  ///@{ @name Movement
  /**
   * @brief Enable camera movement.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_enable_camera_movement(f3d_interactor_t* interactor);

  /**
   * @brief Disable camera movement.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_disable_camera_movement(f3d_interactor_t* interactor);
  ///@}

  ///@{ @name Forwarding input events
  /**
   * @brief Trigger a modifier update.
   *
   * @param interactor Interactor handle.
   * @param mod Input modifier.
   */
  F3D_EXPORT void f3d_interactor_trigger_mod_update(
    f3d_interactor_t* interactor, f3d_interactor_input_modifier_t mod);

  /**
   * @brief Trigger a mouse button event.
   *
   * @param interactor Interactor handle.
   * @param action Input action (press or release).
   * @param button Mouse button.
   */
  F3D_EXPORT void f3d_interactor_trigger_mouse_button(f3d_interactor_t* interactor,
    f3d_interactor_input_action_t action, f3d_interactor_mouse_button_t button);

  /**
   * @brief Trigger a mouse position event.
   *
   * @param interactor Interactor handle.
   * @param xpos X position in window coordinates (pixels).
   * @param ypos Y position in window coordinates (pixels).
   */
  F3D_EXPORT void f3d_interactor_trigger_mouse_position(
    f3d_interactor_t* interactor, double xpos, double ypos);

  /**
   * @brief Trigger a mouse wheel event.
   *
   * @param interactor Interactor handle.
   * @param direction Wheel direction.
   */
  F3D_EXPORT void f3d_interactor_trigger_mouse_wheel(
    f3d_interactor_t* interactor, f3d_interactor_wheel_direction_t direction);

  /**
   * @brief Trigger a keyboard key event.
   *
   * @param interactor Interactor handle.
   * @param action Input action (press or release).
   * @param key_sym Key symbol string.
   */
  F3D_EXPORT void f3d_interactor_trigger_keyboard_key(
    f3d_interactor_t* interactor, f3d_interactor_input_action_t action, const char* key_sym);

  /**
   * @brief Trigger a text character input event.
   *
   * @param interactor Interactor handle.
   * @param codepoint Unicode codepoint of the character.
   */
  F3D_EXPORT void f3d_interactor_trigger_text_character(
    f3d_interactor_t* interactor, unsigned int codepoint);
  ///@}

  /**
   * @brief Manually trigger the event loop.
   *
   * @param interactor Interactor handle.
   * @param delta_time Time step in seconds (must be positive).
   */
  F3D_EXPORT void f3d_interactor_trigger_event_loop(
    f3d_interactor_t* interactor, double delta_time);

  /**
   * @brief Play a VTK interaction file.
   *
   * @param interactor Interactor handle.
   * @param file_path Path to the interaction file.
   * @param delta_time Time step in seconds (default: 1.0/30).
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_interactor_play_interaction(
    f3d_interactor_t* interactor, const char* file_path, double delta_time);

  /**
   * @brief Record interaction to a VTK interaction file.
   *
   * @param interactor Interactor handle.
   * @param file_path Path to save the interaction file.
   * @return 1 on success, 0 on failure.
   */
  F3D_EXPORT int f3d_interactor_record_interaction(
    f3d_interactor_t* interactor, const char* file_path);

  /**
   * @brief Start the interactor event loop.
   *
   * @param interactor Interactor handle.
   * @param delta_time Time step in seconds.
   */
  F3D_EXPORT void f3d_interactor_start(f3d_interactor_t* interactor, double delta_time);

  typedef void (*f3d_interactor_callback_t)(void* user_data);
  /**
   * @brief Start the interactor event loop.
   *
   * @param interactor Interactor handle.
   * @param delta_time Time step in seconds.
   * @param callback Optional user callback called at the start of each event-loop
   *        iteration. May be NULL if no callback is desired.
   * @param user_data Optional opaque pointer passed verbatim to callback.
   */
  F3D_EXPORT void f3d_interactor_start_with_callback(f3d_interactor_t* interactor,
    double delta_time, f3d_interactor_callback_t callback, void* user_data);

  /**
   * @brief Stop the interactor.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_stop(f3d_interactor_t* interactor);

  /**
   * @brief Request a render on the next event loop.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_request_render(f3d_interactor_t* interactor);

  /**
   * @brief Request the interactor to stop on the next event loop.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_request_stop(f3d_interactor_t* interactor);

  /**
   * @brief Free a string array returned by interactor functions.
   *
   * @param array String array to free.
   * @param count Number of strings in the array.
   */
  F3D_EXPORT void f3d_interactor_free_string_array(char** array, int count);

  /**
   * @brief Free a bind array returned by interactor functions.
   *
   * @param array Bind array to free.
   */
  F3D_EXPORT void f3d_interactor_free_bind_array(f3d_interaction_bind_t* array);

#ifdef __cplusplus
}
#endif

#endif // F3D_INTERACTOR_C_API_H
