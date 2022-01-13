/**
 * @class   vtkF3DObjectFactory
 * @brief   A custom factory for VTK classes implemented in F3D
 *
 */
#ifndef vtkF3DObjectFactory_h
#define vtkF3DObjectFactory_h

#include "vtkObjectFactory.h"

class vtkF3DObjectFactory : public vtkObjectFactory
{
public:
  static vtkF3DObjectFactory* New();
  vtkTypeMacro(vtkF3DObjectFactory, vtkObjectFactory);

  const char* GetDescription() override { return "F3D factory overrides."; }

  const char* GetVTKSourceVersion() override;

protected:
  vtkF3DObjectFactory();
  ~vtkF3DObjectFactory() override = default;

private:
  vtkF3DObjectFactory(const vtkF3DObjectFactory&) = delete;
  void operator=(const vtkF3DObjectFactory&) = delete;
};

#endif // vtkF3DObjectFactory_h
