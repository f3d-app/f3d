#include <vtkOpenGLBufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkShader.h>

#include "vtkF3DBitonicSort.h"

#include "vtkF3DBitonicSortFunctions.h"
#include "vtkF3DBitonicSortGlobalDisperseCS.h"
#include "vtkF3DBitonicSortGlobalFlipCS.h"
#include "vtkF3DBitonicSortLocalDisperseCS.h"
#include "vtkF3DBitonicSortLocalSortCS.h"

#include <algorithm>
#include <random>
#include <sstream>

int TestF3DBitonicSort(int argc, char* argv[])
{
  // we need an OpenGL context
  vtkNew<vtkRenderWindow> renWin;
  renWin->OffScreenRenderingOn();
  renWin->Start();

  if (!vtkShader::IsComputeShaderSupported())
  {
    std::cerr << "Compute shaders are not supported on this system, skipping the test.\n";
    return EXIT_SUCCESS;
  }

  constexpr int nbElements = 10000;

  // fill CPU keys and values buffers
  std::vector<double> keys(nbElements);
  std::vector<int> values(nbElements);

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<double> dist(0.0, 1.0);

  std::generate(std::begin(keys), std::end(keys), [&]() { return dist(rng); });
  std::fill(std::begin(values), std::end(values), 0); // we do not care about the values

  // upload these buffers to the GPU
  vtkNew<vtkOpenGLBufferObject> bufferKeys;
  vtkNew<vtkOpenGLBufferObject> bufferValues;

  bufferKeys->Upload(keys, vtkOpenGLBufferObject::ArrayBuffer);
  bufferValues->Upload(values, vtkOpenGLBufferObject::ArrayBuffer);

  // sort
  vtkNew<vtkF3DBitonicSort> sorter;
  sorter->Initialize(128, VTK_DOUBLE, VTK_INT);
  sorter->Run(vtkOpenGLRenderWindow::SafeDownCast(renWin), nbElements, bufferKeys, bufferValues);

  // download sorted key buffer to CPU
  bufferKeys->Download(keys.data(), keys.size());

  // check if correctly sorted
  for (int i = 1; i < nbElements; i++)
  {
    if (keys[i - 1] > keys[i])
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
