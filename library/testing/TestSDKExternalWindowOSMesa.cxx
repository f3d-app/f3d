#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include "engine.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define GLAPI extern
#include <windows.h>
#endif

#include <GL/osmesa.h>

int TestSDKExternalWindowOSMesa([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  int size[] = { 300, 300 };

  // Create an RGBA buffer to hold the rendered image
  std::vector<unsigned char> buffer(size[0] * size[1] * 4);

  std::cout << "Creating OSMesa context..." << std::endl;

  // Create an OSMesa context
  static const int attribs[] = { OSMESA_FORMAT, OSMESA_RGBA, OSMESA_DEPTH_BITS, 32, OSMESA_PROFILE,
    OSMESA_CORE_PROFILE, OSMESA_CONTEXT_MAJOR_VERSION, 3, OSMESA_CONTEXT_MINOR_VERSION, 2, 0 };

  OSMesaContext ctx = OSMesaCreateContextAttribs(attribs, nullptr);
  if (!ctx)
  {
    std::cerr << "OSMesa context creation failed!\n";
    return EXIT_FAILURE;
  }

  std::cout << "Making OSMesa context current..." << std::endl;

  // Bind the buffer to the context
  if (!OSMesaMakeCurrent(ctx, buffer.data(), GL_UNSIGNED_BYTE, size[0], size[1]))
  {
    std::cerr << "OSMesaMakeCurrent failed!\n";
    OSMesaDestroyContext(ctx);
    return EXIT_FAILURE;
  }

  std::cout << "Running F3D..." << std::endl;

  PseudoUnitTest test;

  try
  {
    f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);

    auto getProc = f3d::context::osmesa();
    f3d::context::fptr getString = getProc("glGetString");

    std::cout << "getString address = " << reinterpret_cast<void*>(getString) << std::endl;

    f3d::engine eng = f3d::engine::createExternalOSMesa();
    eng.getWindow().setSize(size[0], size[1]);
    eng.getScene().add(std::string(argv[1]) + "/data/cow.vtp");

    test("render test with external OSMesa window",
      TestSDKHelpers::RenderTest(eng.getWindow(), std::string(argv[1]) + "baselines/", argv[2],
        "TestSDKExternalWindowOSMesa"));
  }
  catch (const f3d::exception& e)
  {
    std::cerr << "OSMesa test failed: " << e.what() << std::endl;
    OSMesaDestroyContext(ctx);
    return EXIT_FAILURE;
  }

  OSMesaDestroyContext(ctx);
  return test.result();
}
