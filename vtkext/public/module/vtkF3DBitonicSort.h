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
   * workgroupSize is the number of threads running in a single GPU workgroup
   * keyType and valueType are the VTK types of the key and value to sort respectively
   * Only VTK_DOUBLE, VTK_FLOAT, VTK_INT and VTK_UNSIGNED_INT are supported
   * Returns true if succeeded
   */
  bool Initialize(int workgroupSize, int keyType, int valueType);

  /**
   * Run the compute shader and sort the buffers.
   * An OpenGL context must exists and given as input in the first argument
   * nbPairs is the number of element in the buffer keys and values
   * OpenGL buffers keys and values must be valid and containing data types specified when
   * this class has been initialized
   * Returns true if succeeded
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
