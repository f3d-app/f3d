/**
 * @class   vtkF3DBitonicSort
 * @brief   Compute shader used to sort key/value pairs
 *
 * This class is used to sort buffers based on the Bitonic Sort algorithm.
 * Inspired by https://poniesandlight.co.uk/reflect/bitonic_merge_sort/.
 * The original code can be found there: https://github.com/tgfrerer/island.
 * It's mostly rewritten but some parts are copied (MIT license, Tim Gfrerer).
 */
#ifndef vtkF3DBitonicSort_h
#define vtkF3DBitonicSort_h

/// @cond
#include <vtkNew.h>
#include <vtkObject.h>
/// @endcond

#include "vtkextModule.h"

class vtkShader;
class vtkShaderProgram;
class vtkOpenGLBufferObject;
class vtkOpenGLRenderWindow;

class VTKEXT_EXPORT vtkF3DBitonicSort : public vtkObject
{
public:
  static vtkF3DBitonicSort* New();
  vtkTypeMacro(vtkF3DBitonicSort, vtkObject);

  /**
   * Initialize the compute shaders.
   * @param workgroupSize the number of threads running in a single GPU workgroup
   * @param keyType the VTK type of the key to sort
   * @param valueType the VTK type of the value to sort
   * Only VTK_DOUBLE, VTK_FLOAT, VTK_INT and VTK_UNSIGNED_INT are supported
   * @return true if succeeded
   */
  bool Initialize(int workgroupSize, int keyType, int valueType);

  /**
   * Run the compute shader and sort the buffers.
   * An OpenGL context must exists and given as input in the first argument
   * @param nbPairs the number of element in the buffer keys and values
   * @param keys OpenGL buffers keys. Must be valid and match data type specified during initialization
   * @param values OpenGL buffers values. Must be valid and match data type specified during initialization
   * @return true if succeeded
   */
  bool Run(vtkOpenGLRenderWindow* context, int nbPairs, vtkOpenGLBufferObject* keys,
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
