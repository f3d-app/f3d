#include "vtkF3DGLTFDocumentLoader.h"

#include <vtkObjectFactory.h>
#include <vtkVersion.h>

#if defined(F3D_MODULE_MESHOPT) && (VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20261109))
#include <meshoptimizer.h>
#endif

namespace
{
// Needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/13979
#if defined(F3D_MODULE_MESHOPT) && (VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20261109))
//----------------------------------------------------------------------------
std::vector<char> DecodeBufferView(
  const vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptCompression& metaData,
  const std::vector<char>& sourceBuffer)
{
  assert(metaData.ByteOffset >= 0);
  assert(metaData.ByteLength > 0);
  assert(metaData.Count > 0);
  assert(metaData.ByteStride > 0);

  const size_t count = metaData.Count;
  const size_t stride = metaData.ByteStride;
  const size_t sourceSize = metaData.ByteLength;
  const unsigned char* source =
    reinterpret_cast<const unsigned char*>(sourceBuffer.data()) + metaData.ByteOffset;

  std::vector<char> decoded(count * stride);
  int ret = -1;

  switch (metaData.Mode)
  {
    case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptMode::ATTRIBUTES:
      ret = meshopt_decodeVertexBuffer(decoded.data(), count, stride, source, sourceSize);
      if (ret == 0)
      {
        switch (metaData.Filter)
        {
          case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptFilter::OCTAHEDRAL:
            meshopt_decodeFilterOct(decoded.data(), count, stride);
            break;
          case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptFilter::QUATERNION:
            meshopt_decodeFilterQuat(decoded.data(), count, stride);
            break;
          case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptFilter::EXPONENTIAL:
            meshopt_decodeFilterExp(decoded.data(), count, stride);
            break;
          case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptFilter::COLOR:
            meshopt_decodeFilterColor(decoded.data(), count, stride);
            break;
          default:
            break;
        }
      }
      break;
    case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptMode::TRIANGLES:
      ret = meshopt_decodeIndexBuffer(decoded.data(), count, stride, source, sourceSize);
      break;
    case vtkGLTFDocumentLoader::BufferView::Extensions::MeshoptMode::INDICES:
      ret = meshopt_decodeIndexSequence(decoded.data(), count, stride, source, sourceSize);
      break;
  }

  return decoded;
}
#endif
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFDocumentLoader);

//----------------------------------------------------------------------------
std::vector<std::string> vtkF3DGLTFDocumentLoader::GetSupportedExtensions()
{
  std::vector<std::string> extensions = this->Superclass::GetSupportedExtensions();
#if defined(F3D_MODULE_MESHOPT) && (VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20261109))
  extensions.emplace_back("EXT_meshopt_compression");
  extensions.emplace_back("KHR_meshopt_compression");
#endif
  return extensions;
}

//----------------------------------------------------------------------------
void vtkF3DGLTFDocumentLoader::PrepareData()
{
  this->Superclass::PrepareData();

#if defined(F3D_MODULE_MESHOPT) && (VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 7, 20261109))
  const std::shared_ptr<Model> model = this->GetInternalModel();

  for (BufferView& bufferView : model->BufferViews)
  {
    const auto& metaData = bufferView.ExtensionMetaData.MeshoptMetaData;

    if (metaData.Buffer >= 0)
    {
      // The buffer view now points to the decoded data, replacing the (optional) fallback buffer
      model->Buffers.emplace_back(::DecodeBufferView(metaData, model->Buffers[metaData.Buffer]));
      bufferView.Buffer = static_cast<int>(model->Buffers.size() - 1);
      bufferView.ByteOffset = 0;
      bufferView.ByteLength = static_cast<int>(model->Buffers.back().size());
      bufferView.ByteStride =
        metaData.Mode == BufferView::Extensions::MeshoptMode::ATTRIBUTES ? metaData.ByteStride : 0;
    }
  }
#endif
}
