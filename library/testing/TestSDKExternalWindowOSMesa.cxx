#include "engine.h"

#include "TestSDKHelpers.h"

#include <GL/osmesa.h>

int TestSDKExternalWindowOSMesa(int argc, char* argv[])
{
  int size[] = { 300, 300 };

  // Create an RGBA buffer to hold the rendered image
  std::vector<unsigned char> buffer(size[0] * size[1] * 4);

  // Create an OSMesa context
  OSMesaContext ctx = OSMesaCreateContext(OSMESA_RGBA, nullptr);
  if (!ctx)
  {
    std::cerr << "OSMesa context creation failed!\n";
    return EXIT_FAILURE;
  }

  // Bind the buffer to the context
  if (!OSMesaMakeCurrent(ctx, buffer.data(), GL_UNSIGNED_BYTE, size[0], size[1]))
  {
    std::cerr << "OSMesaMakeCurrent failed!\n";
    OSMesaDestroyContext(ctx);
    return EXIT_FAILURE;
  }

  f3d::engine eng = f3d::engine::createExternalOSMesa();
  eng.getWindow().setSize(size[0], size[1]);
  eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

  if (!TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2],
        "TestSDKExternalWindowOSMesa"))
  {
    return EXIT_FAILURE;
  }

  OSMesaDestroyContext(ctx);

  return EXIT_SUCCESS;
}
