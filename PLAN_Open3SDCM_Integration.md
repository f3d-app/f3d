# F3D + Open3SDCM Integration Plan: 3Shape DCM File Support

## Executive Summary

Add support for reading 3Shape DCM (Dental CAD Model) files in F3D by integrating the Open3SDCM library as a plugin. This will enable F3D to view dental scans, intraoral scans, and other 3Shape-format 3D models directly.

---

## 1. Repository Analysis

### 1.1 F3D (Current Project)

**Project:** F3D - Fast and minimalist 3D viewer  
**Location:** `/Users/romainnosenzo/CLionProjects/f3d-dcm`  
**Version:** 3.5.0  
**Language:** C++17, VTK-based  
**Architecture:** Plugin-based, modular design

#### Plugin System Architecture

```
plugins/
├── native/           # Built-in plugins (static)
│   ├── CMakeLists.txt
│   ├── module/       # VTK reader/importer implementations
│   │   ├── vtkF3DPLYReader.h/cxx
│   │   └── ...
│   ├── *.inl         # Custom code for readers
│   └── configs/
├── assimp/          # Assimp-based formats
├── occt/            # OpenCASCADE
├── usd/             # Universal Scene Description
└── ...
```

**Key Components:**
- `library/plugin/reader.h` - Base reader class
- `library/plugin/plugin.h` - Base plugin class
- `cmake/f3dPlugin.cmake` - Plugin build macros
- `library/src/factory.cxx.in` - Reader selection logic (template)

**Reader Selection Mechanism:**
```cpp
// From factory.cxx.in
reader* pickReader(plugins, forceReader, isValid) {
  int bestScore = -1;
  reader* bestReader = nullptr;
  for (plugin : plugins) {
    for (reader : plugin->getReaders()) {
      if (reader->getScore() > bestScore && isValid(reader)) {
        bestScore = reader->getScore();
        bestReader = reader;
      }
    }
  }
  return bestReader;
}
```

**Existing DICOM Reader Conflict:**
- Current: `plugins/native/CMakeLists.txt` declares DICOM reader
  - Extension: `.dcm`
  - MIME: `application/dicom`
  - VTK Reader: `vtkDICOMImageReader`
  - Score: 50 (default)

#### Reader Class Interface (from `library/plugin/reader.h`)

```cpp
class reader {
public:
  virtual const std::string getName() const = 0;
  virtual const std::string getShortDescription() const = 0;
  virtual const std::string getLongDescription() const;
  virtual const std::vector<std::string> getExtensions() const = 0;
  virtual const std::vector<std::string> getMimeTypes() const = 0;
  
  virtual bool canRead(const std::string& fileName) const; // Extension-based + stream check
  virtual bool canRead(vtkResourceStream*) const = 0; // Pure stream-based check
  virtual int getScore() const; // Default: 50, range: 0-100
  
  // Geometry reader (VTK algorithm)
  virtual bool hasGeometryReader(); // Default: false
  virtual vtkSmartPointer<vtkAlgorithm> createGeometryReader(const std::string&) const;
  virtual vtkSmartPointer<vtkAlgorithm> createGeometryReader(vtkResourceStream*) const;
  
  // Scene reader (VTK importer)
  virtual bool hasSceneReader(); // Default: false
  virtual vtkSmartPointer<vtkImporter> createSceneReader(const std::string&) const;
  virtual vtkSmartPointer<vtkImporter> createSceneReader(vtkResourceStream*) const;
  
  // Custom hooks
  virtual void applyCustomReader(vtkAlgorithm*, const std::string&, vtkResourceStream*) const;
  virtual void applyCustomImporter(vtkImporter*, const std::string&, vtkResourceStream*) const;
  
  virtual bool supportsStream() const; // Default: false
}
```

---

### 1.2 Open3SDCM Library

**Project:** Open3SDCM - Open-source C++20 library for 3Shape DCM files  
**Location:** `/Users/romainnosenzo/CLionProjects/Open3SDCM`  
**Version:** 1.1.0 (from test data timestamps)  
**License:** MIT-like (see LICENSE file)

#### Library Structure

```
Open3SDCM/
├── Lib/
│   ├── CMakeLists.txt
│   └── src/
│       ├── ParseDcm.h      # Main parser class
│       ├── ParseDcm.cpp    # Implementation (1544 lines)
│       └── definitions.h   # Data structures
├── CLI/                   # Command-line tool
├── TestData/              # Sample DCM files
│   ├── Handle/            # HandleAngledLarge.dcm
│   ├── Hole3x5/           # Hole 3x5.dcm
│   └── real-world/        # scan_019.dcm, scan_040.dcm, scan_045.dcm
└── CMakeLists.txt         # Main project file
```

#### Open3SDCM API (from `ParseDcm.h`)

```cpp
namespace Open3SDCM {
  class DCMParser {
  public:
    void ParseDCM(const std::filesystem::path& filePath);
    bool ExportMesh(const std::filesystem::path& outputPath, 
                    const std::string& format = "stl") const;
    
    // Public data members (filled by ParseDCM)
    std::vector<float> m_Vertices;      // Contiguous x,y,z floats (size/3 = vertex count)
    std::vector<Triangle> m_Triangles;  // Vector of {v1, v2, v3} indices
    SurfaceData m_SurfaceData;         // Colors, textures, metadata
  };
}
```

#### Data Structures (from `definitions.h`)

```cpp
namespace Open3SDCM {
  struct Vertex { float x, y, z; };
  struct Triangle { size_t v1, v2, v3; };
  struct ColorRGB { uint8_t r, g, b; };
  struct TextureCoordinate { float u, v; };
  struct SurfaceData {
    std::optional<ColorRGB> baseColor;
    std::vector<TextureCoordinateData> textureCoordinates;
    std::vector<EmbeddedTextureImage> textureImages;
  };
}
```

#### Supported Features

| Feature | Status | Schema | Since |
|---------|--------|--------|-------|
| Read mesh geometry | ✅ | CA, CB, CC, CE | v0.1.0 |
| Encrypted files (Blowfish) | ✅ | CE | v1.0.0 |
| Per-vertex colors | ✅ | All | v1.1.0 |
| UV mapping | ✅ | All | v1.1.0 |
| Texture images | ✅ | All | v1.1.0 |
| Export to STL | ✅ | All | v0.1.0 |
| Export to PLY | ✅ | All | v0.1.0 |
| Export to OBJ | ✅ | All | v0.1.0 |

#### File Format Details

3Shape DCM files are:
1. **XML text files** (current format) or ZIP archives (older format) containing HPS (Himsa Packed Scan) XML
2. XML starts with `<HPS version="...">` tag - this is the most reliable signature
3. XML contains base64-encoded binary data
4. Binary data includes vertices (float32 x,y,z) and facets (compressed triangle indices)
5. Schema types: CA, CB, CC (unencrypted), CE (encrypted with Blowfish-CBC)
6. Encryption key derived from PackageLockList property via MD5

**User Feedback (2025-05-04):** Current 3Shape DCM files are plain XML text starting with `<HPS version=`, not ZIP files.

#### Dependencies (from `Lib/CMakeLists.txt`)

```cmake
find_package(Poco CONFIG REQUIRED COMPONENTS XML JSON Zip)
find_package(boost_dynamic_bitset CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
```

---

## 2. Integration Strategy

### 2.1 Decision: Plugin Approach

**Chosen:** Create a new plugin `plugins/open3sdcm/`  
**Rationale:** 
- F3D's architecture is plugin-based
- Easy to maintain and update independently
- Can be disabled if not needed
- Follows existing patterns (assimp, occt, etc.)

### 2.2 File Extension Conflict Resolution

**Problem:** `.dcm` extension already used by DICOM reader (medical imaging)  
**Solution Options:**

| Option | Pros | Cons | Decision |
|--------|------|------|----------|
| A. Higher score (70) | Simple, automatic selection | May misdetect DICOM files | ❌ |
| B. Different extension (`.3sdcm`) | No conflict | Requires users to rename files | ❌ |
| C. Custom MIME type | Clean separation | Requires MIME registration | ❌ |
| D. Content-based detection | Most robust | More complex implementation | ✅ |
| E. Both A + D | Best of both worlds | Slightly more complex | ✅ **RECOMMENDED** |

**Recommended:** Use **SCORE 70** + **content-based `canRead()`**  
- Score 70 > DICOM's 50, so 3Shape DCM reader is preferred
- `canRead()` checks for `<HPS version=` or `<HPS` XML signature (primary), or ZIP header (fallback)
- Fallback: If file is actually DICOM, DICOM reader will still work

### 2.3 VTK Reader vs Importer Decision

**Chosen:** `VTK_READER` (geometry only)  
**Rationale:**
- Open3SDCM provides mesh data (vertices + triangles)
- 3Shape DCM files don't contain scene hierarchy, lights, cameras
- Simpler implementation than full importer
- Matches pattern of STL, PLY, OBJ readers in native plugin

### 2.4 Component Design

```
plugins/open3sdcm/
├── CMakeLists.txt          # Plugin configuration
├── open3sdcm.inl          # Custom code (canRead + applyCustomReader)
├── module/
│   ├── CMakeLists.txt
│   ├── vtk.module          # VTK module description
│   ├── vtkF3DOpen3SDCMReader.h
│   └── vtkF3DOpen3SDCMReader.cxx
└── configs/               # Optional: configuration files
```

---

## 3. Implementation Plan

### Phase 1: Plugin Skeleton (30 min)

1. **Create directory structure**
   ```bash
   mkdir -p plugins/open3sdcm/module
   ```

2. **Create `plugins/open3sdcm/CMakeLists.txt`**
   ```cmake
   cmake_minimum_required(VERSION 3.21)
   project(f3d-plugin-open3sdcm)
   
   include(GNUInstallDirs)
   
   if(PROJECT_IS_TOP_LEVEL)
     find_package(f3d REQUIRED COMPONENTS pluginsdk)
   else()
     include(f3dPlugin)
   endif()
   
   # Find Open3SDCM library
   # Option: Add as submodule or external project
   
   f3d_plugin_init()
   
   f3d_plugin_declare_reader(
     NAME Open3SDCM
     EXTENSIONS dcm
     MIMETYPES application/vnd.3shape-dcm
     VTK_READER vtkF3DOpen3SDCMReader
     FORMAT_DESCRIPTION "3Shape DCM (Dental CAD Model)"
     SCORE 70
     CAN_READ CUSTOM
     CUSTOM_CODE "${CMAKE_CURRENT_SOURCE_DIR}/open3sdcm.inl"
   )
   
   f3d_plugin_build(
     NAME open3sdcm
     VERSION 1.0
     DESCRIPTION "3Shape DCM file format support"
     VTK_MODULES IOGeometry
   )
   ```

### Phase 2: VTK Reader Implementation (1 hour)

**File: `plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.h`**

```cpp
#ifndef vtkF3DOpen3SDCMReader_h
#define vtkF3DOpen3SDCMReader_h

#include <vtkPolyDataAlgorithm.h>

class vtkF3DOpen3SDCMReader : public vtkPolyDataAlgorithm
{
public:
  static vtkF3DOpen3SDCMReader* New();
  vtkTypeMacro(vtkF3DOpen3SDCMReader, vtkPolyDataAlgorithm);

  // File to read
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // For stream reading
  vtkSetObjectMacro(ResourceStream);
  vtkGetObjectMacro(ResourceStream, vtkResourceStream);

  int CanReadFile(const std::string& filename);

protected:
  vtkF3DOpen3SDCMReader();
  ~vtkF3DOpen3SDCMReader() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  char* FileName;
  vtkResourceStream* ResourceStream;

private:
  vtkF3DOpen3SDCMReader(const vtkF3DOpen3SDCMReader&) = delete;
  void operator=(const vtkF3DOpen3SDCMReader&) = delete;
};

#endif
```

**File: `plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.cxx`**

```cpp
#include "vtkF3DOpen3SDCMReader.h"

#include <Open3SDCM/ParseDcm.h>

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(vtkF3DOpen3SDCMReader);

vtkF3DOpen3SDCMReader::vtkF3DOpen3SDCMReader()
  : FileName(nullptr)
  , ResourceStream(nullptr)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkF3DOpen3SDCMReader::~vtkF3DOpen3SDCMReader()
{
  this->SetFileName(nullptr);
  this->SetResourceStream(nullptr);
}

int vtkF3DOpen3SDCMReader::CanReadFile(const std::string& filename)
{
  // Check if file is a valid 3Shape DCM
  // Implementation: Try to parse and check for 3Shape schema
  return Open3SDCM::DCMParser::Is3ShapeDCM(filename);
}

int vtkF3DOpen3SDCMReader::RequestInformation(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* vtkNotUsed(outputVector))
{
  // Set output type
  vtkInformation* outInfo = vtkInformationVector::GetOutputInformation(outputVector, 0);
  outInfo->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

int vtkF3DOpen3SDCMReader::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  vtkPolyData* output = vtkPolyData::GetData(outputVector);
  
  if (!output)
  {
    return 0;
  }

  Open3SDCM::DCMParser parser;
  
  if (this->ResourceStream)
  {
    // Stream-based reading - need to save to temp file
    // Open3SDCM requires file path, not stream
    return 0; // Not supported yet
  }
  
  if (!this->FileName || !this->FileName[0])
  {
    vtkErrorMacro("No filename specified");
    return 0;
  }

  parser.ParseDCM(this->FileName);
  
  if (parser.m_Vertices.empty() || parser.m_Triangles.empty())
  {
    vtkErrorMacro("Failed to parse DCM file: " << this->FileName);
    return 0;
  }

  // Create points
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(parser.m_Vertices.size() / 3);
  
  float* ptr = static_cast<float*>(points->GetVoidPointer(0));
  std::copy(parser.m_Vertices.begin(), parser.m_Vertices.end(), ptr);
  
  output->SetPoints(points);

  // Create cells (triangles)
  vtkNew<vtkCellArray> cells;
  cells->SetNumberOfCells(parser.m_Triangles.size());
  
  for (const auto& tri : parser.m_Triangles)
  {
    cells->InsertNextCell(3);
    cells->InsertCellPoint(tri.v1);
    cells->InsertCellPoint(tri.v2);
    cells->InsertCellPoint(tri.v3);
  }
  
  output->SetPolys(cells);

  // Add colors if available
  if (parser.m_SurfaceData.baseColor)
  {
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetName("Colors");
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(points->GetNumberOfPoints());
    
    const auto& color = *parser.m_SurfaceData.baseColor;
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
      colors->SetTuple3(i, color.r, color.g, color.b);
    }
    
    output->GetPointData()->SetScalars(colors);
  }

  // Add per-vertex colors from texture coordinates if available
  // TODO: Implement texture coordinate mapping

  return 1;
}
```

### Phase 3: Custom Code for Reader (30 min)

**File: `plugins/open3sdcm/open3sdcm.inl`**

```cpp
#include <Open3SDCM/ParseDcm.h>

#include <vtkFileResourceStream.h>

bool canRead(vtkResourceStream* stream) const override
{
  // 3Shape DCM files are ZIP archives with specific structure
  // Check for PK ZIP header
  const char* header = "PK\x03\x04";
  
  if (!stream || !stream->Open()) {
    return false;
  }
  
  // Read first 4 bytes
  char buffer[4];
  if (stream->Read(buffer, 4) != 4) {
    stream->Close();
    return false;
  }
  
  stream->Close();
  
  // Check for ZIP magic
  if (memcmp(buffer, header, 4) != 0) {
    return false;
  }
  
  // For more accurate detection, we could:
  // 1. Parse the XML to check for HPS Schema element
  // 2. Look for CA/CB/CC/CE schema tags
  // But this is complex for canRead(), so we'll use the XML signature check
  
  return true;
}

bool canRead(const std::string& fileName) const override
{
  // First check extension
  std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
  if (ext != "dcm") {
    return false;
  }
  
  // Then try to detect if it's a 3Shape DCM vs DICOM
  // 3Shape DCM files are XML starting with "<HPS version=" or "<HPS"
  // DICOM files start with "DICM"
  std::ifstream file(fileName, std::ios::binary);
  if (!file) {
    return false;
  }
  
  char header[16];
  if (!file.read(header, 16)) {
    return false;
  }
  
  // DICOM files start with "DICM"
  const char dicomHeader[] = { 'D', 'I', 'C', 'M' };
  if (memcmp(header, dicomHeader, 4) == 0) {
    return false; // It's DICOM, not 3Shape
  }
  
  // 3Shape DCM files start with "<HPS version=" or "<HPS"
  const char hpsHeaderFull[] = "<HPS version=";
  const char hpsHeaderShort[] = "<HPS";
  if (memcmp(header, hpsHeaderFull, 12) == 0 || memcmp(header, hpsHeaderShort, 5) == 0) {
    return true; // It's 3Shape DCM (XML format)
  }
  
  // Fallback: Some older 3Shape DCM files might be ZIP archives
  const char zipHeader[] = { 'P', 'K', '\x03', '\x04' };
  if (memcmp(header, zipHeader, 4) == 0) {
    return true; // It's a ZIP, could be 3Shape DCM (older format)
  }
  
  // Unknown - let other readers try
  return false;
}
```

### Phase 4: Open3SDCM Dependency Integration (30 min)

**Options:**

#### Option A: Git Submodule (Recommended for production)
```cmake
# In f3d-dcm root CMakeLists.txt
add_subdirectory(external/Open3SDCM)
```

#### Option B: FetchContent (Good for CI)
```cmake
# In plugins/open3sdcm/CMakeLists.txt
include(FetchContent)
FetchContent_Declare(
  Open3SDCM
  GIT_REPOSITORY https://github.com/Nosenzor/Open3SDCM.git
  GIT_TAG master
)
FetchContent_MakeAvailable(Open3SDCM)
```

#### Option C: Local Path (For development)
```cmake
# In plugins/open3sdcm/CMakeLists.txt
set(Open3SDCM_DIR /Users/romainnosenzo/CLionProjects/Open3SDCM)
add_subdirectory(${Open3SDCM_DIR} external/Open3SDCM)
```

**Dependencies needed:**
- Poco (XML, Zip) - Already available in F3D? Need to check
- OpenSSL - Already available in F3D? Need to check
- Assimp - Already used by F3D for other plugins
- Boost.dynamic_bitset - Need to check

### Phase 5: CMake Integration

Need to ensure all dependencies are properly linked. Let me check what F3D already has:

```cmake
# In plugins/open3sdcm/CMakeLists.txt
find_package(Poco CONFIG REQUIRED COMPONENTS XML Zip)
find_package(OpenSSL REQUIRED)
find_package(assimp REQUIRED)
find_package(Boost REQUIRED COMPONENTS dynamic_bitset)

# Link to Open3SDCM library
target_link_libraries(vtkF3DOpen3SDCMReader PRIVATE Open3SDCMLib)
```

---

## 4. Critical Thinking: Potential Issues & Mitigations

### 4.1 Extension Conflict (.dcm)

**Issue:** Both DICOM (medical) and 3Shape DCM use `.dcm` extension  
**Analysis:**
- DICOM files: Binary format, start with "DICM" magic bytes
- 3Shape DCM files: ZIP archives, start with "PK\x03\x04" magic bytes
- These are fundamentally different file types

**Solution:**
1. Use SCORE 70 for 3Shape DCM (higher than DICOM's 50)
2. Implement `canRead()` that checks magic bytes
3. If ZIP header → 3Shape DCM (score 70)
4. If DICOM header → DICOM reader (score 50)
5. Result: Correct reader selected automatically

**Verification:**
```cpp
// Test with 3Shape DCM file
Open3SDCM::DCMParser parser;
parser.ParseDCM("test.dcm");
// Should succeed

// Test with DICOM file  
vtkNew<vtkDICOMImageReader> reader;
reader->SetFileName("dicom.dcm");
reader->Update();
// Should succeed
```

### 4.2 Open3SDCM Dependencies

**Issue:** Open3SDCM requires Poco, OpenSSL, Assimp, Boost  
**Analysis:**
- F3D already uses VTK which may have some of these
- Need to verify which dependencies are already available

**Dependencies Check:**

| Dependency | Open3SDCM Usage | F3D Status | Action |
|------------|-----------------|------------|--------|
| Poco::XML | XML parsing | ❓ | Add to plugin |
| Poco::Zip | ZIP extraction | ❓ | Add to plugin |
| OpenSSL | Blowfish decryption | ❓ | Add to plugin |
| Assimp | Mesh export | ✅ (used by assimp plugin) | Reuse |
| Boost.dynamic_bitset | Bit operations | ❓ | Add to plugin |

**Solution:** Add all required dependencies to plugin's CMakeLists.txt

### 4.3 Stream Support

**Issue:** Open3SDCM::DCMParser::ParseDCM() takes a file path, not a stream  
**Analysis:**
- Open3SDCM uses Poco::Zip::Decompress which requires file paths
- VTK supports stream-based reading for some readers

**Solution:**
1. For now: Disable stream support (`SUPPORTS_STREAM` not set)
2. Future: Modify Open3SDCM to accept std::istream or memory buffer
3. Alternative: Save stream to temp file, parse, then delete

**Implementation:**
```cmake
f3d_plugin_declare_reader(
  ...
  # SUPPORTS_STREAM not included - disable for now
)
```

### 4.4 Memory Management

**Issue:** Open3SDCM returns raw vectors, need to convert to VTK data structures  
**Analysis:**
- `m_Vertices`: std::vector<float> → vtkPoints (float array)
- `m_Triangles`: std::vector<Triangle> → vtkCellArray
- `m_SurfaceData`: Optional colors/textures → vtkPointData arrays

**Solution:** Direct memory copy where possible:
```cpp
float* ptr = static_cast<float*>(points->GetVoidPointer(0));
std::copy(parser.m_Vertices.begin(), parser.m_Vertices.end(), ptr);
```

### 4.5 Error Handling

**Issue:** Open3SDCM uses exceptions and std::cerr  
**Analysis:**
- Open3SDCM catches Poco::Exception and std::exception
- VTK expects vtkErrorMacro for errors

**Solution:**
```cpp
try {
  parser.ParseDCM(filename);
} catch (const std::exception& e) {
  vtkErrorMacro("Failed to parse DCM: " << e.what());
  return 0;
}
```

### 4.6 Build System Integration

**Issue:** Open3SDCM uses C++20, F3D uses C++17?  
**Analysis:** Need to check F3D's C++ standard

**Check F3D C++ Standard:**
```bash
# From f3d-dcm/CMakeLists.txt
grep -i "CMAKE_CXX_STANDARD" CMakeLists.txt
```

If F3D uses C++17 and Open3SDCM requires C++20:
- Solution: Set C++20 for the plugin only
```cmake
target_compile_features(vtkF3DOpen3SDCMReader PRIVATE cxx_std_20)
```

### 4.7 Cross-Platform Issues

**Issue:** File paths, endianness, etc.  
**Analysis:**
- Open3SDCM uses std::filesystem (cross-platform)
- Open3SDCM handles endianness for CE schema (SwapEndianness)
- VTK is cross-platform

**Solution:** No special handling needed

### 4.8 Testing Strategy

**Test Files Available:**
- `/Users/romainnosenzo/CLionProjects/Open3SDCM/TestData/Handle/HandleAngledLarge.dcm`
- `/Users/romainnosenzo/CLionProjects/Open3SDCM/TestData/Hole3x5/Hole 3x5.dcm`
- `/Users/romainnosenzo/CLionProjects/Open3SDCM/TestData/real-world/*.dcm`

**Test Plan:**
1. Build F3D with open3sdcm plugin
2. Test loading each DCM file
3. Verify geometry is displayed correctly
4. Verify colors are displayed (if present)
5. Test with DICOM file to ensure no conflict

---

## 5. Updated Implementation Checklist

### Pre-Implementation (Current)
- [x] Analyze F3D plugin system
- [x] Analyze Open3SDCM library API
- [x] Identify extension conflict
- [x] Design integration approach
- [x] Identify critical issues and mitigations

### Implementation
- [ ] Create `plugins/open3sdcm/` directory structure
- [ ] Create `plugins/open3sdcm/CMakeLists.txt`
- [ ] Create `plugins/open3sdcm/module/CMakeLists.txt`
- [ ] Create `plugins/open3sdcm/module/vtk.module`
- [ ] Create `plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.h`
- [ ] Create `plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.cxx`
- [ ] Create `plugins/open3sdcm/open3sdcm.inl`
- [ ] Add Open3SDCM as dependency (submodule or external)
- [ ] Update F3D's main CMakeLists.txt to include plugin
- [ ] Build and test

### Post-Implementation
- [ ] Test with all sample DCM files
- [ ] Test DICOM still works
- [ ] Add to CI build
- [ ] Documentation
- [ ] Create pull request

---

## 6. File Locations Summary

| File | Location | Status |
|------|----------|--------|
| PLAN_Open3SDCM_Integration.md | f3d-dcm/PLAN_Open3SDCM_Integration.md | ✅ Created |
| plugins/open3sdcm/CMakeLists.txt | f3d-dcm/plugins/open3sdcm/CMakeLists.txt | ⏳ |
| plugins/open3sdcm/module/CMakeLists.txt | f3d-dcm/... | ⏳ |
| plugins/open3sdcm/module/vtk.module | f3d-dcm/... | ⏳ |
| plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.h | f3d-dcm/... | ⏳ |
| plugins/open3sdcm/module/vtkF3DOpen3SDCMReader.cxx | f3d-dcm/... | ⏳ |
| plugins/open3sdcm/open3sdcm.inl | f3d-dcm/... | ⏳ |

---

## 7. Estimated Timeline

| Phase | Duration | Complexity |
|-------|----------|------------|
| Phase 1: Skeleton | 30 min | Low |
| Phase 2: VTK Reader | 1 hour | Medium |
| Phase 3: Custom Code | 30 min | Medium |
| Phase 4: Dependencies | 30 min | Medium |
| Phase 5: CMake Integration | 30 min | Medium |
| Testing & Debugging | 2-4 hours | High |
| **Total** | **4-6 hours** | |

---

## 8. Success Criteria

1. ✅ F3D can open and display 3Shape DCM files
2. ✅ Geometry (vertices + triangles) is correctly rendered
3. ✅ Colors are displayed if present in file
4. ✅ DICOM files still work (no regression)
5. ✅ Plugin builds successfully on Linux, macOS, Windows
6. ✅ No memory leaks or crashes

---

## 9. References

- [F3D GitHub](https://github.com/f3d-app/f3d)
- [Open3SDCM GitHub](https://github.com/Nosenzor/Open3SDCM)
- [3Shape DCM Format Documentation](Packed Scan Standard format 501.pdf)
- VTK Documentation: vtkPolyDataAlgorithm, vtkPoints, vtkCellArray

---

*Plan created: 2025-05-03*  
*Author: Mistral Vibe (with user input)*
