#include <context_c_api.h>

int test_context_c_api()
{
    // none of the functions should throw exception
    f3d_context_glx();
    f3d_context_wgl();
    f3d_context_cocoa();
    f3d_context_egl();
    f3d_context_osmesa();

    return 0;
}
