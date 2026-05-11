#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <mesh_view.h>
#include <scene.h>
#include <window.h>

int TestSDKSceneFromMemoryZeroCopyExceptions([[maybe_unused]] int argc,
                                             [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();

  // Mesh view must have points
  class NoPointMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      return { .pointCount = 0 };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with no point", [&]() { sce.add(std::make_shared<NoPointMesh>()); });

  // Mesh view points data pointer is null
  class NullPointDataMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      return { .pointCount = 1, .points = { .data = nullptr } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with null point data", [&]() { sce.add(std::make_shared<NullPointDataMesh>()); });

  // Mesh view points must have a data type of F32 or F64
  class InvalidPointTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1, .points = { .type = data_type::U8, .data = dummyPointer } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid point type", [&]() { sce.add(std::make_shared<InvalidPointTypeMesh>()); });

  // Mesh view points must have 3 components
  class InvalidPointComponentCountMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1, .points = { .data = dummyPointer, .components = 4 } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>("add with invalid point component count", [&]() {
    sce.add(std::make_shared<InvalidPointComponentCountMesh>());
  });

  // Mesh view normals must have a data type of F32 or F64
  class InvalidNormalTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .normals = { .type = data_type::U8, .data = dummyPointer } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid normal type", [&]() { sce.add(std::make_shared<InvalidNormalTypeMesh>()); });

  // Mesh view normals must have 3 components
  class InvalidNormalComponentCountMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .normals = { .data = dummyPointer, .components = 4 } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>("add with invalid normal component count", [&]() {
    sce.add(std::make_shared<InvalidNormalComponentCountMesh>());
  });

  // Mesh view texture coordinates must have a data type of F32 or F64
  class InvalidTextureCoordinateTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .textureCoordinates = { .type = data_type::U8, .data = dummyPointer } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid texture coordinate type",
    [&]() { sce.add(std::make_shared<InvalidTextureCoordinateTypeMesh>()); });

  // Mesh view texture coordinates must have 2 components
  class InvalidTextureCoordinateComponentCountMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .textureCoordinates = { .data = dummyPointer, .components = 3 } };
    }
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with invalid texture coordinate component count",
    [&]() { sce.add(std::make_shared<InvalidTextureCoordinateComponentCountMesh>()); });

  // Mesh view face offsets pointer is null
  class NullFaceOffsetsMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .faceOffsetCount = 1,
               .faceOffsets = { .data = nullptr } };
    }
  };

  // Mesh view face indices pointer is null
  class NullFaceIndicesMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .faceOffsetCount = 2,
               .faceOffsets = { .data = dummyPointer },
               .faceIndexCount = 1,
               .faceIndices = { .data = nullptr } };
    };
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with null face offsets", [&]() { sce.add(std::make_shared<NullFaceOffsetsMesh>()); });

  // Mesh view face offsets must have a data type of I32 or I64
  class InvalidFaceOffsetsTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .faceOffsetCount = 2,
               .faceOffsets = { .type = data_type::U8, .data = dummyPointer },
               .faceIndexCount = 1,
               .faceIndices = { .data = dummyPointer } };
    };
  };

  test.expect<f3d::scene::load_failure_exception>("add with invalid face offsets type", [&]() {
    sce.add(std::make_shared<InvalidFaceOffsetsTypeMesh>());
  });

  // Mesh view face indices must have a data type of I32 or I64
  class InvalidFaceIndicesTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .faceOffsetCount = 2,
               .faceOffsets = { .type = data_type::I32, .data = dummyPointer },
               .faceIndexCount = 1,
               .faceIndices = { .type = data_type::U8, .data = dummyPointer } };
    };
  };

  test.expect<f3d::scene::load_failure_exception>("add with invalid face indices type", [&]() {
    sce.add(std::make_shared<InvalidFaceIndicesTypeMesh>());
  });

  // Mesh view face offsets and face indices must have the same data type
  class FaceOffsetsIndicesMismatchedTypeMesh : public f3d::mesh_view
  {
  public:
    f3d::mesh_view::memory_view_t getMemoryView(double) const override
    {
      const void* dummyPointer = reinterpret_cast<const void*>(0xf3d);
      return { .pointCount = 1,
               .points = { .data = dummyPointer, .components = 3 },
               .faceOffsetCount = 2,
               .faceOffsets = { .type = data_type::I32, .data = dummyPointer },
               .faceIndexCount = 1,
               .faceIndices = { .type = data_type::I64, .data = dummyPointer } };
    };
  };

  test.expect<f3d::scene::load_failure_exception>(
    "add with mismatched face offsets and indices type",
    [&]() { sce.add(std::make_shared<FaceOffsetsIndicesMismatchedTypeMesh>()); });

  return test.result();
}
