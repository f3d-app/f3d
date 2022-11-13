#include "init.h"

#include "log.h"

#include "vtkF3DObjectFactory.h"

#include <vtkLogger.h>
#include <vtkNew.h>
#include <vtkVersion.h>

namespace f3d::detail
{
class init::internals
{
};

//----------------------------------------------------------------------------
init::init()
  : Internals(std::make_unique<init::internals>())
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
}

//----------------------------------------------------------------------------
init::~init() = default;

}
