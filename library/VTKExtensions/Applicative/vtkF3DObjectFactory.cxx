#include "vtkF3DObjectFactory.h"

#include "vtkF3DConfigure.h"
#include "vtkF3DPolyDataMapper.h"

#ifdef __ANDROID__
#include "vtkF3DAndroidLogOutputWindow.h"
#elif F3D_WINDOWS_GUI
#include "vtkF3DWin32OutputWindow.h"
#else
#include "vtkF3DConsoleOutputWindow.h"
#endif

#include <vtkVersion.h>

vtkStandardNewMacro(vtkF3DObjectFactory);

// Now create the functions to create overrides with.
VTK_CREATE_CREATE_FUNCTION(vtkF3DPolyDataMapper)

#ifdef __ANDROID__
VTK_CREATE_CREATE_FUNCTION(vtkF3DAndroidLogOutputWindow)
#elif F3D_WINDOWS_GUI
VTK_CREATE_CREATE_FUNCTION(vtkF3DWin32OutputWindow)
#else
VTK_CREATE_CREATE_FUNCTION(vtkF3DConsoleOutputWindow)
#endif

//----------------------------------------------------------------------------
vtkF3DObjectFactory::vtkF3DObjectFactory()
{
  this->RegisterOverride("vtkPolyDataMapper", "vtkF3DPolyDataMapper",
    "vtkPolyDataMapper override for F3D", 1, vtkObjectFactoryCreatevtkF3DPolyDataMapper);

#ifdef __ANDROID__
  this->RegisterOverride("vtkOutputWindow", "vtkF3DAndroidLogOutputWindow",
    "vtkOutputWindow override for F3D", 1, vtkObjectFactoryCreatevtkF3DAndroidLogOutputWindow);
#elif F3D_WINDOWS_GUI
  this->RegisterOverride("vtkOutputWindow", "vtkF3DWin32OutputWindow",
    "vtkOutputWindow override for F3D", 1, vtkObjectFactoryCreatevtkF3DWin32OutputWindow);
#else
  this->RegisterOverride("vtkOutputWindow", "vtkF3DConsoleOutputWindow",
    "vtkOutputWindow override for F3D", 1, vtkObjectFactoryCreatevtkF3DConsoleOutputWindow);
#endif
}

//----------------------------------------------------------------------------
const char* vtkF3DObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}
