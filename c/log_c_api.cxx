#include "log_c_api.h"
#include "log.h"
#include <string>

static f3d_log_forward_fn_t g_log_callback = nullptr;

//----------------------------------------------------------------------------
static void cpp_log_forwarder(f3d::log::VerboseLevel level, const std::string& message)
{
  if (g_log_callback != nullptr)
  {
    f3d_log_verbose_level_t c_level = static_cast<f3d_log_verbose_level_t>(level);
    g_log_callback(c_level, message.c_str());
  }
}

//----------------------------------------------------------------------------
void f3d_log_set_verbose_level(f3d_log_verbose_level_t level, int force_std_err)
{
  f3d::log::VerboseLevel cpp_level = static_cast<f3d::log::VerboseLevel>(level);
  f3d::log::setVerboseLevel(cpp_level, force_std_err != 0);
}

//----------------------------------------------------------------------------
f3d_log_verbose_level_t f3d_log_get_verbose_level(void)
{
  f3d::log::VerboseLevel cpp_level = f3d::log::getVerboseLevel();
  return static_cast<f3d_log_verbose_level_t>(cpp_level);
}

//----------------------------------------------------------------------------
void f3d_log_set_use_coloring(int use)
{
  f3d::log::setUseColoring(use != 0);
}

//----------------------------------------------------------------------------
void f3d_log_print(f3d_log_verbose_level_t level, const char* message)
{
  if (!message)
  {
    return;
  }

  f3d::log::VerboseLevel cpp_level = static_cast<f3d::log::VerboseLevel>(level);
  f3d::log::print(cpp_level, message);
}

//----------------------------------------------------------------------------
void f3d_log_debug(const char* message)
{
  if (!message)
  {
    return;
  }

  f3d::log::debug(message);
}

//----------------------------------------------------------------------------
void f3d_log_info(const char* message)
{
  if (!message)
  {
    return;
  }

  f3d::log::info(message);
}

//----------------------------------------------------------------------------
void f3d_log_warn(const char* message)
{
  if (!message)
  {
    return;
  }

  f3d::log::warn(message);
}

//----------------------------------------------------------------------------
void f3d_log_error(const char* message)
{
  if (!message)
  {
    return;
  }

  f3d::log::error(message);
}

//----------------------------------------------------------------------------
void f3d_log_forward(f3d_log_forward_fn_t callback)
{
  g_log_callback = callback;

  if (callback != nullptr)
  {
    f3d::log::forward(cpp_log_forwarder);
  }
  else
  {
    f3d::log::forward(nullptr);
  }
}
