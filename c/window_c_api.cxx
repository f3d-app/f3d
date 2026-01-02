#include "window_c_api.h"
#include "image.h"
#include "window.h"

//----------------------------------------------------------------------------
f3d_window_type_t f3d_window_get_type(f3d_window_t* window)
{
  if (!window)
  {
    return F3D_WINDOW_UNKNOWN;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  f3d::window::Type type = cpp_window->getType();
  return static_cast<f3d_window_type_t>(type);
}

//----------------------------------------------------------------------------
int f3d_window_is_offscreen(f3d_window_t* window)
{
  if (!window)
  {
    return 0;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  return cpp_window->isOffscreen() ? 1 : 0;
}

//----------------------------------------------------------------------------
f3d_camera_t* f3d_window_get_camera(f3d_window_t* window)
{
  if (!window)
  {
    return nullptr;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  f3d::camera& camera = cpp_window->getCamera();
  return reinterpret_cast<f3d_camera_t*>(&camera);
}

//----------------------------------------------------------------------------
int f3d_window_render(f3d_window_t* window)
{
  if (!window)
  {
    return 0;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  return cpp_window->render() ? 1 : 0;
}

//----------------------------------------------------------------------------
f3d_image_t* f3d_window_render_to_image(f3d_window_t* window, int no_background)
{
  if (!window)
  {
    return nullptr;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  f3d::image img = cpp_window->renderToImage(no_background != 0);

  f3d::image* heap_img = new f3d::image(std::move(img));
  return reinterpret_cast<f3d_image_t*>(heap_img);
}

//----------------------------------------------------------------------------
void f3d_window_set_size(f3d_window_t* window, int width, int height)
{
  if (!window)
  {
    return;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  cpp_window->setSize(width, height);
}

//----------------------------------------------------------------------------
int f3d_window_get_width(const f3d_window_t* window)
{
  if (!window)
  {
    return 0;
  }

  const f3d::window* cpp_window = reinterpret_cast<const f3d::window*>(window);
  return cpp_window->getWidth();
}

//----------------------------------------------------------------------------
int f3d_window_get_height(const f3d_window_t* window)
{
  if (!window)
  {
    return 0;
  }

  const f3d::window* cpp_window = reinterpret_cast<const f3d::window*>(window);
  return cpp_window->getHeight();
}

//----------------------------------------------------------------------------
void f3d_window_set_position(f3d_window_t* window, int x, int y)
{
  if (!window)
  {
    return;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  cpp_window->setPosition(x, y);
}

//----------------------------------------------------------------------------
void f3d_window_set_icon(f3d_window_t* window, const unsigned char* icon, size_t icon_size)
{
  if (!window || !icon)
  {
    return;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  cpp_window->setIcon(icon, icon_size);
}

//----------------------------------------------------------------------------
void f3d_window_set_window_name(f3d_window_t* window, const char* window_name)
{
  if (!window || !window_name)
  {
    return;
  }

  f3d::window* cpp_window = reinterpret_cast<f3d::window*>(window);
  cpp_window->setWindowName(window_name);
}

//----------------------------------------------------------------------------
void f3d_window_get_world_from_display(
  const f3d_window_t* window, const f3d_point3_t display_point, f3d_point3_t world_point)
{
  if (!window || !display_point || !world_point)
  {
    return;
  }

  const f3d::window* cpp_window = reinterpret_cast<const f3d::window*>(window);
  f3d::point3_t cpp_display_point = { display_point[0], display_point[1], display_point[2] };
  f3d::point3_t cpp_world_point = cpp_window->getWorldFromDisplay(cpp_display_point);
  world_point[0] = cpp_world_point[0];
  world_point[1] = cpp_world_point[1];
  world_point[2] = cpp_world_point[2];
}

//----------------------------------------------------------------------------
void f3d_window_get_display_from_world(
  const f3d_window_t* window, const f3d_point3_t world_point, f3d_point3_t display_point)
{
  if (!window || !world_point || !display_point)
  {
    return;
  }

  const f3d::window* cpp_window = reinterpret_cast<const f3d::window*>(window);
  f3d::point3_t cpp_world_point = { world_point[0], world_point[1], world_point[2] };
  f3d::point3_t cpp_display_point = cpp_window->getDisplayFromWorld(cpp_world_point);
  display_point[0] = cpp_display_point[0];
  display_point[1] = cpp_display_point[1];
  display_point[2] = cpp_display_point[2];
}
