#include "PseudoUnitTest.h"
#include "TestSDKHelpers.h"

#include <engine.h>
#include <interactor.h>
#include <log.h>
#include <mesh_view.h>
#include <scene.h>
#include <window.h>

struct WavyGridMesh
{
  // SoA structure holding different vertex information
  // Used to test strided memory
  struct Vertex
  {
    float position[3] = { 0.0, 0.0, 0.0 };
    float normal[3] = { 0.0, 0.0, 1.0 };
    float uv[2] = { 0.0, 0.0 };
    float velocity = 0.0;
  };

  WavyGridMesh(unsigned int nx, unsigned int ny)
  {
    // Initialize flat grid
    constexpr float size = 1.0f;

    const size_t vertexCount = (nx + 1) * (ny + 1);
    const size_t quadCount = nx * ny;

    this->Vertices.resize(vertexCount);

    for (unsigned int j = 0; j <= ny; ++j)
    {
      for (unsigned int i = 0; i <= nx; ++i)
      {
        float u = static_cast<float>(i) / nx;
        float v = static_cast<float>(j) / ny;

        Vertex& vert = this->Vertices[j * (nx + 1) + i];

        vert.position[0] = (u - 0.5f) * size * 2.0f;
        vert.position[1] = (v - 0.5f) * size * 2.0f;
        vert.position[2] = 0.f;
        vert.normal[0] = 0.f;
        vert.normal[1] = 0.f;
        vert.normal[2] = 1.f;
        vert.uv[0] = u;
        vert.uv[1] = v;
      }
    }

    // Initialize quads
    this->Quads.reserve(quadCount * 4);
    this->QuadCoords.reserve(quadCount * 2);
    for (unsigned int j = 0; j < ny; ++j)
    {
      for (unsigned int i = 0; i < nx; ++i)
      {
        unsigned int i0 = j * (nx + 1) + i;
        unsigned int i1 = j * (nx + 1) + (i + 1);
        unsigned int i2 = (j + 1) * (nx + 1) + (i + 1);
        unsigned int i3 = (j + 1) * (nx + 1) + i;

        this->Quads.push_back(i0);
        this->Quads.push_back(i1);
        this->Quads.push_back(i2);
        this->Quads.push_back(i3);

        this->QuadCoords.push_back(static_cast<unsigned short>(i));
        this->QuadCoords.push_back(static_cast<unsigned short>(j));
      }
    }

    // Only quads
    this->FaceOffsets.resize(this->Quads.size() / 4 + 1);
    std::generate(this->FaceOffsets.begin(), this->FaceOffsets.end(), [n = 0]() mutable {
      const unsigned int offset = n;
      n += 4;
      return offset;
    });

    // Random for coverage
    this->DummyU8.resize(quadCount);
    this->DummyI8.resize(quadCount);
    this->DummyI16.resize(quadCount);
    this->DummyU32.resize(quadCount);
    this->DummyI32.resize(quadCount);
    this->DummyU64.resize(quadCount);
    this->DummyI64.resize(quadCount);
    this->DummyF64.resize(quadCount);
    for (size_t i = 0; i < quadCount; ++i)
    {
      this->DummyU8[i] = static_cast<uint8_t>(i % 256);
      this->DummyI8[i] = -static_cast<int8_t>(i % 128);
      this->DummyI16[i] = -static_cast<int16_t>(i * 10);
      this->DummyU32[i] = static_cast<uint32_t>(i * 1000);
      this->DummyI32[i] = -static_cast<int32_t>(i * 1000);
      this->DummyU64[i] = static_cast<uint64_t>(i * 1000000);
      this->DummyI64[i] = -static_cast<int64_t>(i * 1000000);
      this->DummyF64[i] = static_cast<double>(i) * 0.0000001;
    }
    this->Update(0.0f);
  }

  // update time-varying attributes
  void Update(float time)
  {
    // Update height field
    constexpr float amplitude = 0.1f;
    constexpr float freq = 4.0f;

    for (Vertex& vert : this->Vertices)
    {
      // Height field
      vert.position[2] = amplitude * std::sin(freq * vert.position[0] + time) *
        std::cos(freq * vert.position[1] + time);

      // Analytical normal computation
      float dzdx = amplitude * freq * std::cos(freq * vert.position[0] + time) *
        std::cos(freq * vert.position[1] + time);

      float dzdy = amplitude * freq * std::sin(freq * vert.position[0] + time) *
        -std::sin(freq * vert.position[1] + time);

      float dzdt = amplitude *
        (std::cos(freq * vert.position[0] + time) * std::cos(freq * vert.position[1] + time) +
          std::sin(freq * vert.position[0] + time) * -std::sin(freq * vert.position[1] + time));

      // Normal = normalize(-dz/dx, -dz/dy, 1)
      const float nxv = -dzdx;
      const float nyv = -dzdy;
      constexpr float nzv = 1.0f;

      float len = std::sqrt(nxv * nxv + nyv * nyv + nzv * nzv);

      vert.normal[0] = nxv / len;
      vert.normal[1] = nyv / len;
      vert.normal[2] = nzv / len;

      vert.velocity = std::abs(dzdt);
    }
  }

  std::vector<Vertex> Vertices;
  std::vector<unsigned int> Quads;
  std::vector<unsigned int> FaceOffsets;

  // Cell scalar with 2 short components
  std::vector<unsigned short> QuadCoords;

  // for coverage
  std::vector<uint8_t> DummyU8;
  std::vector<int8_t> DummyI8;
  std::vector<int16_t> DummyI16;
  std::vector<uint32_t> DummyU32;
  std::vector<int32_t> DummyI32;
  std::vector<uint64_t> DummyU64;
  std::vector<int64_t> DummyI64;
  std::vector<double> DummyF64;
};

// This wrapper tests static mesh with no normals
class StaticZeroCopyMesh : public f3d::mesh_view
{
public:
  explicit StaticZeroCopyMesh(const WavyGridMesh& grid)
    : Grid(grid)
  {
  }

  f3d::mesh_view::memory_view_t getMemoryView(double) const override
  {
    const float* points = reinterpret_cast<const float*>(this->Grid.Vertices.data());
    size_t stride = sizeof(WavyGridMesh::Vertex) / sizeof(float);

    return { .pointCount = this->Grid.Vertices.size(),
      .points = { .data = points, .components = 3, .stride = stride },
      .textureCoordinates = { .data = points + 6, .components = 2, .stride = stride },
      .faceOffsetCount = this->Grid.FaceOffsets.size(),
      .faceOffsets = { .type = f3d::mesh_view::data_type::I32, .data = this->Grid.FaceOffsets.data() },
      .faceIndexCount = this->Grid.Quads.size(),
      .faceIndices = { .type = f3d::mesh_view::data_type::I32, .data = this->Grid.Quads.data() } };
  }

private:
  const WavyGridMesh& Grid;
};

// This wrapper tests animated mesh with normals and scalars but no texture coordinates
class AnimatedZeroCopyMesh : public f3d::mesh_view
{
public:
  explicit AnimatedZeroCopyMesh(const WavyGridMesh& grid)
    : Grid(grid)
  {
  }

  std::array<double, 2> getTimeRange() const override
  {
    return { 0.0, 10.0 };
  }

  f3d::mesh_view::memory_view_t getMemoryView(double) const override
  {
    const float* points = reinterpret_cast<const float*>(this->Grid.Vertices.data());
    size_t stride = sizeof(WavyGridMesh::Vertex) / sizeof(float);

    return { .pointCount = this->Grid.Vertices.size(),
             .points = { .data = points, .components = 3, .stride = stride },
             .normals = { .data = points + 3, .components = 3, .stride = stride },
             .faceOffsetCount = this->Grid.FaceOffsets.size(),
             .faceOffsets = { .type = f3d::mesh_view::data_type::I32, .data = this->Grid.FaceOffsets.data() },
             .faceIndexCount = this->Grid.Quads.size(),
             .faceIndices = { .type = f3d::mesh_view::data_type::I32, .data = this->Grid.Quads.data() },
             .pointScalars = { { .name = "velocity", .data = points + 8, .stride = stride },
                                },
             .faceScalars = { { .name = "quad_coords",
                                .type = f3d::mesh_view::data_type::U16,
                                .data = this->Grid.QuadCoords.data(),
                                .components = 2,
                                .stride = 2 },
                                { .name = "dummy_u8",
                                 .type = f3d::mesh_view::data_type::U8,
                                 .data = this->Grid.DummyU8.data() },
                               { .name = "dummy_i8",
                                 .type = f3d::mesh_view::data_type::I8,
                                 .data = this->Grid.DummyI8.data() },
                               { .name = "dummy_i16",
                                 .type = f3d::mesh_view::data_type::I16,
                                 .data = this->Grid.DummyI16.data() },
                               { .name = "dummy_u32",
                                 .type = f3d::mesh_view::data_type::U32,
                                 .data = this->Grid.DummyU32.data() },
                               { .name = "dummy_i32",
                                 .type = f3d::mesh_view::data_type::I32,
                                 .data = this->Grid.DummyI32.data() },
                               { .name = "dummy_u64",
                                 .type = f3d::mesh_view::data_type::U64,
                                 .data = this->Grid.DummyU64.data() },
                               { .name = "dummy_i64",
                                 .type = f3d::mesh_view::data_type::I64,
                                 .data = this->Grid.DummyI64.data() },
                               { .name = "dummy_f64",
                                 .type = f3d::mesh_view::data_type::F64,
                                 .data = this->Grid.DummyF64.data() } } };
  }

private:
  const WavyGridMesh& Grid;
};

int TestSDKSceneFromMemoryZeroCopy([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  PseudoUnitTest test;

  f3d::log::setVerboseLevel(f3d::log::VerboseLevel::DEBUG);
  f3d::engine eng = f3d::engine::create(true);
  f3d::scene& sce = eng.getScene();
  f3d::interactor& inter = eng.getInteractor();
  f3d::window& win = eng.getWindow().setSize(500, 500);
  win.setWindowName("Zero-copy animated mesh");

  eng.getOptions().ui.animation_progress = true;
  eng.getOptions().render.show_edges = true;
  eng.getOptions().model.scivis.enable = true;
  eng.getOptions().model.scivis.cells = true;
  eng.getOptions().ui.scalar_bar = true;

  WavyGridMesh grid(20, 20);

  // Add static mesh from memory
  test(
    "add static mesh from memory", [&]() { sce.add(std::make_shared<StaticZeroCopyMesh>(grid)); });

  std::string texturePath = std::string(argv[1]) + "data/world.png";
  eng.getOptions().model.color.texture = texturePath;

  test("render textured static mesh",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", argv[2],
      "TestSDKSceneFromMemoryZeroCopyTexturedStaticMesh"));

  eng.getOptions().model.color.texture = std::nullopt;

  test("clear the scene", [&]() { sce.clear(); });

  // Add animated mesh from memory (zero-copy)
  test("add animated mesh from memory",
    [&]() { sce.add(std::make_shared<AnimatedZeroCopyMesh>(grid)); });

  // update mesh data on event loop callback
  inter.setEventLoopUserCallback([&grid](f3d::interactor_state_t state) {
    grid.Update(static_cast<float>(state.animationTime));
  });

  // Check dummy ranges for coverage
  eng.getOptions().model.scivis.array_name = "dummy_u8";
  test("render mesh dummy_u8",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", argv[2],
      "TestSDKSceneFromMemoryZeroCopyDummyU8", 0.07)); // bug for uchars

  eng.getOptions().model.scivis.array_name = "dummy_i8";
  test("render mesh dummy_i8",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyI8"));

  eng.getOptions().model.scivis.array_name = "dummy_i16";
  test("render mesh dummy_i16",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyI16"));

  eng.getOptions().model.scivis.array_name = "dummy_u32";
  test("render mesh dummy_u32",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyU32"));

  eng.getOptions().model.scivis.array_name = "dummy_i32";
  test("render mesh dummy_i32",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyI32"));

  eng.getOptions().model.scivis.array_name = "dummy_u64";
  test("render mesh dummy_u64",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyU64"));

  eng.getOptions().model.scivis.array_name = "dummy_i64";
  test("render mesh dummy_i64",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyI64"));

  eng.getOptions().model.scivis.array_name = "dummy_f64";
  test("render mesh dummy_f64",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyDummyF64"));

  // Start animation
  inter.startAnimation();
  inter.triggerEventLoop(1.0); // advance animation by 1 second

  eng.getOptions().model.scivis.array_name = "velocity";
  eng.getOptions().model.scivis.cells = false;

  // Render test
  test("render mesh at time 1.0",
    TestSDKHelpers::RenderTest(
      win, std::string(argv[1]) + "baselines/", argv[2], "TestSDKSceneFromMemoryZeroCopyVelocity"));

  inter.triggerEventLoop(1.0); // advance animation by 1 second

  eng.getOptions().model.scivis.array_name = "quad_coords";
  eng.getOptions().model.scivis.cells = true;
  eng.getOptions().model.scivis.component = 0;

  // Render test
  test("render mesh at time 2.0",
    TestSDKHelpers::RenderTest(win, std::string(argv[1]) + "baselines/", argv[2],
      "TestSDKSceneFromMemoryZeroCopyQuadCoords"));

  return test.result();
}
