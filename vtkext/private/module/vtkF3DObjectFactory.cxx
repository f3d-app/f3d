#include "vtkF3DObjectFactory.h"

#include <vtkVersion.h>

#include "vtkF3DPolyDataMapper.h"

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) &&                                           \
  VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240203)
#include <vtkF3DPointSplatMapper.h>
#endif

#ifdef __ANDROID__
#include <vtkF3DAndroidLogOutputWindow.h>
#else
#include <vtkF3DConsoleOutputWindow.h>
#endif

#if F3D_MODULE_UI
#include "vtkF3DImguiActor.h"
#include "vtkF3DImguiConsole.h"
#include "vtkF3DImguiObserver.h"
#endif

namespace
{
template<typename T>
vtkObject* Factory()
{
  vtkObject* obj = T::New();
  return obj;
}
}

vtkStandardNewMacro(vtkF3DObjectFactory);

//----------------------------------------------------------------------------
vtkF3DObjectFactory::vtkF3DObjectFactory()
{
  this->RegisterOverride("vtkPolyDataMapper", "vtkF3DPolyDataMapper",
    "vtkPolyDataMapper override for F3D", 1, ::Factory<vtkF3DPolyDataMapper>);

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) &&                                           \
  VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 3, 20240203)
  this->RegisterOverride("vtkPointGaussianMapper", "vtkF3DPointSplatMapper",
    "vtkPointGaussianMapper override for F3D", 1, ::Factory<vtkF3DPointSplatMapper>);
#endif

#ifdef __ANDROID__
  this->RegisterOverride("vtkOutputWindow", "vtkF3DAndroidLogOutputWindow",
    "vtkOutputWindow override for F3D", 1, ::Factory<vtkF3DAndroidLogOutputWindow>);
#elif F3D_MODULE_UI
  this->RegisterOverride("vtkOutputWindow", "vtkF3DImguiConsole",
    "vtkOutputWindow override for F3D", 1, ::Factory<vtkF3DImguiConsole>);
#else
  this->RegisterOverride("vtkOutputWindow", "vtkF3DConsoleOutputWindow",
    "vtkOutputWindow override for F3D", 1, ::Factory<vtkF3DConsoleOutputWindow>);
#endif

#if F3D_MODULE_UI
  this->RegisterOverride("vtkF3DUIActor", "vtkF3DImguiActor", "vtkF3DUIActor override for F3D", 1,
    ::Factory<vtkF3DImguiActor>);
  this->RegisterOverride("vtkF3DUIObserver", "vtkF3DImguiObserver",
    "vtkF3DUIObserver override for F3D", 1, ::Factory<vtkF3DImguiObserver>);
#endif
}

//----------------------------------------------------------------------------
const char* vtkF3DObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}
