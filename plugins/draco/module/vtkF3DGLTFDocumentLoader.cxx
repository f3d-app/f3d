#include "vtkF3DGLTFDocumentLoader.h"

#include <vtkObjectFactory.h>

#include <algorithm>

#include "draco/compression/decode.h"

namespace
{

//----------------------------------------------------------------------------
template<typename T>
std::vector<char> DecodeIndexBuffer(const std::unique_ptr<draco::Mesh>& mesh)
{
  std::vector<char> outBuffer(mesh->num_faces() * 3 * sizeof(T));

  for (draco::FaceIndex f(0); f < mesh->num_faces(); ++f)
  {
    const draco::Mesh::Face& face = mesh->face(f);

    T indices[3] = { static_cast<T>(face[0].value()), static_cast<T>(face[1].value()),
      static_cast<T>(face[2].value()) };

    std::copy(
      indices, indices + 3, reinterpret_cast<T*>(outBuffer.data() + f.value() * sizeof(indices)));
  }

  return outBuffer;
}

//----------------------------------------------------------------------------
std::vector<char> DecodeIndexBuffer(
  const std::unique_ptr<draco::Mesh>& mesh, vtkGLTFDocumentLoader::ComponentType compType)
{
  // indexing using float does not make sense
  assert(compType != vtkGLTFDocumentLoader::ComponentType::FLOAT);

  switch (compType)
  {
    case vtkGLTFDocumentLoader::ComponentType::BYTE:
      return DecodeIndexBuffer<int8_t>(mesh);
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_BYTE:
      return DecodeIndexBuffer<uint16_t>(mesh);
    case vtkGLTFDocumentLoader::ComponentType::SHORT:
      return DecodeIndexBuffer<int16_t>(mesh);
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_SHORT:
      return DecodeIndexBuffer<uint16_t>(mesh);
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_INT:
      return DecodeIndexBuffer<uint32_t>(mesh);
    default:
      break;
  }

  return {};
}

//----------------------------------------------------------------------------
template<typename T>
std::vector<char> DecodeVertexBuffer(
  const std::unique_ptr<draco::Mesh>& mesh, const draco::PointAttribute* attribute)
{
  std::vector<char> outBuffer(mesh->num_points() * attribute->num_components() * sizeof(T));
  std::vector<T> values(attribute->num_components());

  size_t byteOffset = 0;
  for (draco::PointIndex i(0); i < mesh->num_points(); ++i)
  {
    attribute->ConvertValue<T>(
      attribute->mapped_index(i), attribute->num_components(), values.data());

    std::copy(values.begin(), values.end(), reinterpret_cast<T*>(outBuffer.data() + byteOffset));

    byteOffset += sizeof(T) * attribute->num_components();
  }

  return outBuffer;
}

//----------------------------------------------------------------------------
std::vector<char> DecodeVertexBuffer(vtkGLTFDocumentLoader::ComponentType compType,
  const std::unique_ptr<draco::Mesh>& mesh, int attIndex)
{
  const draco::PointAttribute* attribute = mesh->GetAttributeByUniqueId(attIndex);
  switch (compType)
  {
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_BYTE:
      return DecodeVertexBuffer<uint8_t>(mesh, attribute);
    case vtkGLTFDocumentLoader::ComponentType::BYTE:
      return DecodeVertexBuffer<int8_t>(mesh, attribute);
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_SHORT:
      return DecodeVertexBuffer<uint16_t>(mesh, attribute);
    case vtkGLTFDocumentLoader::ComponentType::SHORT:
      return DecodeVertexBuffer<int16_t>(mesh, attribute);
    case vtkGLTFDocumentLoader::ComponentType::UNSIGNED_INT:
      return DecodeVertexBuffer<uint32_t>(mesh, attribute);
    case vtkGLTFDocumentLoader::ComponentType::FLOAT:
      return DecodeVertexBuffer<float>(mesh, attribute);
  }

  return {};
}
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkF3DGLTFDocumentLoader);

//----------------------------------------------------------------------------
std::vector<std::string> vtkF3DGLTFDocumentLoader::GetSupportedExtensions()
{
  std::vector<std::string> extensions = this->Superclass::GetSupportedExtensions();
  extensions.emplace_back("KHR_draco_mesh_compression");
  return extensions;
}

//----------------------------------------------------------------------------
void vtkF3DGLTFDocumentLoader::PrepareData()
{
  std::shared_ptr<Model> model = this->GetInternalModel();

  for (size_t i = 0; i < model->Meshes.size(); i++)
  {
    for (Primitive& primitive : model->Meshes[i].Primitives)
    {
      // check if Draco metadata is present
      auto& dracoMetaData = primitive.ExtensionMetaData.KHRDracoMetaData;
      if (dracoMetaData.BufferView >= 0)
      {
        auto& view = model->BufferViews[primitive.ExtensionMetaData.KHRDracoMetaData.BufferView];
        auto& buffer = model->Buffers[view.Buffer];

        draco::DecoderBuffer decoderBuffer;
        decoderBuffer.Init(buffer.data() + view.ByteOffset, view.ByteLength);
        auto decodeResult = draco::Decoder().DecodeMeshFromBuffer(&decoderBuffer);
        if (decodeResult.ok())
        {
          const std::unique_ptr<draco::Mesh>& mesh = decodeResult.value();

          auto& accessor = model->Accessors[primitive.IndicesId];

          // handle index buffer
          if (primitive.Indices >= 0)
          {
            model->Buffers.emplace_back(::DecodeIndexBuffer(mesh, accessor.ComponentTypeValue));

            vtkGLTFDocumentLoader::BufferView decodedIndexBufferView;
            decodedIndexBufferView.Buffer = static_cast<int>(model->Buffers.size() - 1);
            decodedIndexBufferView.ByteLength = model->Buffers.back().size();
            decodedIndexBufferView.ByteOffset = 0;
            decodedIndexBufferView.ByteStride = 0;
            decodedIndexBufferView.Target =
              static_cast<int>(vtkGLTFDocumentLoader::Target::ARRAY_BUFFER);
            model->BufferViews.emplace_back(std::move(decodedIndexBufferView));

            accessor.BufferView = static_cast<int>(model->BufferViews.size() - 1);
            accessor.Count = static_cast<int>(mesh->num_faces() * 3);
          }

          // handle vertex attributes
          for (const auto& attrib : dracoMetaData.AttributeIndices)
          {
            auto& attrAccessor = model->Accessors[primitive.AttributeIndices[attrib.first]];

            model->Buffers.emplace_back(
              ::DecodeVertexBuffer(attrAccessor.ComponentTypeValue, mesh, attrib.second));

            vtkGLTFDocumentLoader::BufferView decodedBufferView;
            decodedBufferView.Buffer = static_cast<int>(model->Buffers.size() - 1);
            decodedBufferView.ByteLength = model->Buffers.back().size();
            decodedBufferView.ByteOffset = 0;
            decodedBufferView.ByteStride = 0;
            decodedBufferView.Target =
              static_cast<int>(vtkGLTFDocumentLoader::Target::ELEMENT_ARRAY_BUFFER);
            model->BufferViews.emplace_back(std::move(decodedBufferView));

            attrAccessor.BufferView = static_cast<int>(model->BufferViews.size() - 1);
            attrAccessor.Count = static_cast<int>(mesh->num_points());
            attrAccessor.ByteOffset = 0;
          }
        }
      }
    }
  }
}
