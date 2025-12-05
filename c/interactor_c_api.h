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

  ///@{ @name Animation
  /**
   * @brief Toggle the animation.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_toggle_animation(f3d_interactor_t* interactor);

  /**
   * @brief Start the animation.
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_start_animation(f3d_interactor_t* interactor);

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

  ///@{ @name Commands
  /**
   * @brief Initialize commands (remove existing and add defaults).
   *
   * @param interactor Interactor handle.
   */
  F3D_EXPORT void f3d_interactor_init_commands(f3d_interactor_t* interactor);

  /**
   * @brief Remove a command for the provided action.
   *
   * @param interactor Interactor handle.
   * @param action Action string.
   */
  F3D_EXPORT void f3d_interactor_remove_command(f3d_interactor_t* interactor, const char* action);

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
   * @brief Remove a binding for the provided bind.
   *
   * @param interactor Interactor handle.
   * @param bind Interaction bind.
   */
  F3D_EXPORT void f3d_interactor_remove_binding(
    f3d_interactor_t* interactor, const f3d_interaction_bind_t* bind);

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

#ifdef __cplusplus
}
#endif

#endif // F3D_INTERACTOR_C_API_H
