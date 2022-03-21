#include "f3d_init.h"

#include "f3d_log.h"

#include "F3DReaderInstantiator.h"
#include "vtkF3DObjectFactory.h"

#include <vtkLogger.h>
#include <vtkNew.h>
#include <vtkVersion.h>

namespace f3d
{
class init::F3DInternals
{
public:
  F3DReaderInstantiator ReaderInstantiator;
};

//----------------------------------------------------------------------------
init::init()
  : Internals(new init::F3DInternals())
{
#if NDEBUG
  vtkObject::GlobalWarningDisplayOff();
#endif

  // Disable vtkLogger in case VTK was built with log support
  vtkLogger::SetStderrVerbosity(vtkLogger::VERBOSITY_OFF);
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200701)
  vtkLogger::SetInternalVerbosityLevel(vtkLogger::VERBOSITY_OFF);
#endif

  // instanciate our own polydata mapper and output windows
  vtkNew<vtkF3DObjectFactory> factory;
  vtkObjectFactory::RegisterFactory(factory);
  vtkObjectFactory::SetAllEnableFlags(0, "vtkPolyDataMapper", "vtkOpenGLPolyDataMapper");

  // Make sure to initialize the output window
  // after the object factory and before the first usage.
  f3d::log::setQuiet(false);
}
}

const f3d::init gInitInstance;
