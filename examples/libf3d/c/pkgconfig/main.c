#include <f3d/engine_c_api.h>
#include <f3d/log_c_api.h>

int main(void)
{
  f3d_engine_autoload_plugins();

  f3d_engine_t* engine = f3d_engine_create_none();

  f3d_log_info("F3D engine is loaded");

  f3d_engine_delete(engine);

  return 0;
}
