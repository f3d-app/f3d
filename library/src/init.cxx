#include "init.h"

#include "log.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DObjectFactory.h"

#if F3D_MODULE_EXR
#include "vtkF3DEXRReader.h"
#endif

#include <vtkImageReader2Factory.h>
#include <vtkLogger.h>
#include <vtkNew.h>
#include <vtkVersion.h>

#include <memory>

namespace f3d::detail
{

//----------------------------------------------------------------------------
void init::initialize()
{
  static std::unique_ptr<init> instance;
  if (!instance)
  {
    instance = std::make_unique<init>();
  }
}

//----------------------------------------------------------------------------
init::init()
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // Disable vtkLogger in case VTK was built with log support
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
// SetInternalVerbosityLevel needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7078
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200701)
  vtkLogger::SetInternalVerbosityLevel(vtkLogger::VERBOSITY_OFF);
#endif

  // instantiate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

#ifdef __EMSCRIPTEN__
  vtkObjectFactory::SetAllEnableFlags(0, "vtkRenderWindow", "vtkOpenGLRenderWindow");
  vtkObjectFactory::SetAllEnableFlags(
    0, "vtkRenderWindowInteractor", "vtkGenericRenderWindowInteractor");
#endif

#if F3D_MODULE_EXR
  vtkNew<vtkF3DEXRReader> reader;
  vtkImageReader2Factory::RegisterReader(reader);
#endif
}
}
