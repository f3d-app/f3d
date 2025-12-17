#include <log_c_api.h>

#include <stdio.h>
#include <string.h>

static int g_callback_count = 0;
static f3d_log_verbose_level_t g_last_level = F3D_LOG_INFO;
static char g_last_message[256] = { 0 };

static void test_log_callback(f3d_log_verbose_level_t level, const char* message)
{
  g_callback_count++;
  g_last_level = level;
  if (message)
  {
    strncpy(g_last_message, message, sizeof(g_last_message) - 1);
    g_last_message[sizeof(g_last_message) - 1] = '\0';
  }
}

int test_log_c_api()
{
  int failed = 0;

  f3d_log_verbose_level_t initial_level = f3d_log_get_verbose_level();

  f3d_log_set_verbose_level(F3D_LOG_DEBUG, 0);
  if (f3d_log_get_verbose_level() != F3D_LOG_DEBUG)
  {
    puts("[ERROR] Expected F3D_LOG_DEBUG");
    failed++;
  }

  f3d_log_set_verbose_level(F3D_LOG_QUIET, 0);
  if (f3d_log_get_verbose_level() != F3D_LOG_QUIET)
  {
    puts("[ERROR] Expected F3D_LOG_QUIET");
    failed++;
  }

  f3d_log_set_verbose_level(initial_level, 0);

  f3d_log_set_use_coloring(1);
  f3d_log_set_use_coloring(0);

  f3d_log_verbose_level_t saved_level = f3d_log_get_verbose_level();
  f3d_log_set_verbose_level(F3D_LOG_QUIET, 0);

  f3d_log_print(F3D_LOG_DEBUG, "Test debug message");
  f3d_log_print(F3D_LOG_INFO, "Test info message");
  f3d_log_print(F3D_LOG_WARN, "Test warning message");
  f3d_log_print(F3D_LOG_ERROR, "Test error message");

  f3d_log_set_verbose_level(saved_level, 0);

  g_callback_count = 0;
  g_last_level = F3D_LOG_INFO;
  memset(g_last_message, 0, sizeof(g_last_message));

  f3d_log_forward(test_log_callback);
  f3d_log_print(F3D_LOG_WARN, "Test callback message");

  if (g_callback_count != 1)
  {
    puts("[ERROR] Callback should have been called once");
    failed++;
  }

  if (g_last_level != F3D_LOG_WARN)
  {
    puts("[ERROR] Expected callback level F3D_LOG_WARN");
    failed++;
  }

  if (strcmp(g_last_message, "Test callback message") != 0)
  {
    puts("[ERROR] Expected callback message 'Test callback message'");
    failed++;
  }

  f3d_log_debug(NULL);
  f3d_log_warn(NULL);
  f3d_log_info(NULL);
  f3d_log_error(NULL);
  f3d_log_forward(NULL);

  return failed;
}
