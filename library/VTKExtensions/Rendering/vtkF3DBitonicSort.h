/**
 * @class   vtkF3DBitonicSort
 * @brief   Compute shader used to sort key/value pairs
 *
 * This class is used to sort buffers based on the Bitonic Sort algorithm.
 * Inspired by https://poniesandlight.co.uk/reflect/bitonic_merge_sort/
 * The original code can be found there: https://github.com/tgfrerer/island
 * It's mostly rewritten but some parts are copied (MIT license, Tim Gfrerer)
 */
#ifndef vtkF3DBitonicSort_h
#define vtkF3DBitonicSort_h

#include <vtkNew.h>
#include <vtkObject.h>

class vtkShader;
class vtkShaderProgram;
class vtkOpenGLBufferObject;
class vtkOpenGLRenderWindow;

class vtkF3DBitonicSort : public vtkObject
{
public:
  static vtkF3DBitonicSort* New();
  vtkTypeMacro(vtkF3DBitonicSort, vtkObject);

  void Initialize(int workgroupSize, int keyType, int valueType);

  void Run(vtkOpenGLRenderWindow* context, int nbPairs, vtkOpenGLBufferObject* keys,
    vtkOpenGLBufferObject* values);

private:
  vtkNew<vtkShader> BitonicSortLocalSortComputeShader;
  vtkNew<vtkShaderProgram> BitonicSortLocalSortProgram;
  vtkNew<vtkShader> BitonicSortLocalDisperseComputeShader;
  vtkNew<vtkShaderProgram> BitonicSortLocalDisperseProgram;
  vtkNew<vtkShader> BitonicSortGlobalFlipComputeShader;
  vtkNew<vtkShaderProgram> BitonicSortGlobalFlipProgram;
  vtkNew<vtkShader> BitonicSortGlobalDisperseComputeShader;
  vtkNew<vtkShaderProgram> BitonicSortGlobalDisperseProgram;

  int WorkgroupSize = -1;
};

#endif
