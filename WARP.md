# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

F3D is a fast and minimalist 3D viewer desktop application with a C++17 API, supporting many file formats including glTF, USD, STL, STEP, PLY, OBJ, FBX, and Alembic. It includes both an application and libf3d library with Python, Java, and JavaScript bindings.

## Common Development Commands

### Building F3D

**Basic build:**
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ../

# Build (Windows PowerShell)
cmake --build .

# Build (Linux/macOS)
make -j$(nproc)
```

**Build with all features enabled:**
```bash
cmake .. \
  -DBUILD_TESTING=ON \
  -DF3D_MODULE_RAYTRACING=ON \
  -DF3D_MODULE_EXR=ON \
  -DF3D_MODULE_UI=ON \
  -DF3D_PLUGIN_BUILD_ALEMBIC=ON \
  -DF3D_PLUGIN_BUILD_ASSIMP=ON \
  -DF3D_PLUGIN_BUILD_DRACO=ON \
  -DF3D_PLUGIN_BUILD_OCCT=ON \
  -DF3D_PLUGIN_BUILD_USD=ON \
  -DF3D_PLUGIN_BUILD_VDB=ON \
  -DF3D_BINDINGS_PYTHON=ON
```

**Windows-specific build with Visual Studio:**
```bash
# Use x64 Native Tools Command Prompt for VS 2022
cmake .. -DVTK_DIR=C:/path/to/vtk/install/lib/cmake/vtk-9.5
cmake --build . --config Debug
```

### Testing

**Run all tests:**
```bash
# Build first
cmake --build .

# Run tests
ctest
```

**Run specific test:**
```bash
ctest -R TestPLY
```

**Run tests with verbose output:**
```bash
ctest -V
```

**Run specific test categories:**
```bash
# Run only rendering tests
ctest -R "Test.*" -E "NoRender"

# Run interaction tests
ctest -R ".*Interaction.*"
```

**Enable different test categories during configuration:**
```bash
cmake .. \
  -DBUILD_TESTING=ON \
  -DF3D_TESTING_ENABLE_RENDERING_TESTS=ON \
  -DF3D_TESTING_ENABLE_LONG_TIMEOUT_TESTS=ON \
  -DF3D_TESTING_ENABLE_GLX_TESTS=ON
```

### Code Quality

**Format code with clang-format:**
```bash
# Single file
clang-format -i /path/to/file.cxx

# All files (Linux/macOS)
shopt -s globstar; clang-format -i **/*.{h,cxx}
```

**Format Python code:**
```bash
# Single file
black /path/to/file.py

# All Python files
black --include '(\.py|\.py\.in)' .
```

**Format other files (markdown, JSON, etc.):**
```bash
# Single file
prettier -w /path/to/file.md

# All files (Linux/macOS)
shopt -s dotglob; shopt -s globstar; prettier -w **/*.{js,json,md,html,yml}
```

### Running F3D

**Basic usage:**
```bash
# From build directory
./bin/f3d /path/to/file.ext

# Save rendering to image
./bin/f3d /path/to/file.ext --output=/path/to/output.png
```

**Common rendering options:**
```bash
# Enable wireframe edges and grid
./bin/f3d model.stl --edges --grid

# Volume rendering
./bin/f3d volume.vti --volume

# Scalar coloring with color bar
./bin/f3d data.vtu --scalar-coloring --scalar-bar

# Raytracing (requires raytracing module)
./bin/f3d model.obj --raytracing --raytracing-samples=50
```

## Architecture Overview

F3D follows a modular architecture with clear separation of concerns:

### Core Components

**Application Layer (`application/`):**
- `F3DStarter`: Main application logic and startup
- `F3DOptionsTools`: Command-line options handling
- `F3DConfigFileTools`: Configuration file management
- Entry point (`main.cxx`)

**Library Layer (`library/`):**
- `libf3d`: Core C++17 API for 3D rendering
- Split into public API headers (`public/`) and private implementation (`private/`)
- Options system defined in `options.json` (generates all configuration code)
- Supports multiple bindings: Python, Java, JavaScript

**Plugin System (`plugins/`):**
- Modular file format support
- Can be built statically (embedded) or dynamically loaded
- Built-in plugins:
  - `native`: Basic VTK formats (STL, PLY, OBJ, VTK, etc.)
  - `hdf`: VTKHDF, ExodusII, NetCDF (enabled by default)
  - `alembic`: ABC files (optional, requires Alembic)
  - `assimp`: FBX, DAE, OFF, DXF, X, 3MF (optional, requires Assimp)
  - `draco`: DRC files (optional, requires Draco)
  - `occt`: STEP, IGES, BREP, XBF (optional, requires OpenCASCADE)
  - `usd`: USD files (optional, requires OpenUSD)
  - `vdb`: VDB files (optional, requires OpenVDB)

**VTK Extensions (`vtkext/`):**
- `public`: Classes available to plugin developers (`vtkF3DImporter` base class)
- `private`: Internal rendering classes (`vtkF3DRenderer`, UI components)

### Configuration System

F3D supports a sophisticated configuration system with JSON files:

**Configuration hierarchy:**
1. Built-in defaults (from `options.json`)
2. System-wide configuration files
3. User configuration files  
4. Command-line arguments

**Configuration structure:**
```json
{
  "options": {
    "scene.up_direction": "+Z",
    "render.show_edges": true,
    "ui.scalar_bar": true
  },
  "bindings": {
    "Ctrl+E": "toggle render.show_edges",
    "G": "toggle render.grid.enable"
  }
}
```

**File-specific configurations:**
```json
[
  {
    "match-type": "glob",
    "match": "*.stl",
    "options": {
      "scene.up_direction": "+Z",
      "render.grid.enable": true
    }
  }
]
```

### Key Options Categories

From `library/options.json`, the main option categories are:

- **scene**: Camera settings, animations, up direction
- **render**: Visual effects (edges, grid, raytracing, anti-aliasing)
- **ui**: User interface elements (axis, scalar bar, filename display)
- **model**: Material and texture properties, point sprites, volume rendering
- **interactor**: Interaction behavior (trackball mode, zoom inversion)

## Development Guidelines

### Code Style

**C++ Formatting:**
- CamelCase naming
- Two-space indentation
- Curly braces on new lines
- `//----------------------------------------------------------------------------` before methods in implementation

**Component-specific conventions:**
- **Application classes**: Start with `F3D`, methods start with uppercase
- **libf3d classes**: Start with lowercase, methods start with lowercase  
- **VTK extensions**: Follow VTK conventions, start with `vtkF3D` if inheriting from vtkObject

### Testing Strategy

F3D has comprehensive testing across multiple layers:

**Application Layer:**
- Image comparison tests using reference baselines
- Interaction tests with recorded user actions
- Command-line option validation

**Library Layer:**  
- C++ unit tests for libf3d API
- Cross-platform functionality testing

**Bindings Layer:**
- Language-specific tests (Python, Java, JavaScript)
- API consistency validation

**VTK Extensions Layer:**
- Low-level VTK functionality tests
- Rendering pipeline validation

### Adding New Tests

**For basic functionality:**
```cmake
f3d_test(NAME TestNewFeature DATA testfile.obj ARGS --new-option)
```

**For interaction tests:**
1. Record interaction: `f3d --interaction-test-record TestName.log`
2. Copy to `testing/recordings/TestName.log`
3. Add test: `f3d_test(NAME TestName DATA file.ext INTERACTION)`

**For UI tests:**
```cmake
f3d_test(NAME TestUI DATA file.ext ARGS --show-ui UI)
```

### Plugin Development

**Creating a new plugin:**
1. Add directory under `plugins/`
2. Inherit from `vtkF3DImporter` (from public vtkext module)
3. Implement required virtual methods
4. Add CMake configuration with `F3D_PLUGIN_BUILD_YOURPLUGIN` option
5. Add tests for supported file formats

**Plugin structure:**
```cpp
class vtkF3DYourPluginImporter : public vtkF3DImporter
{
  bool CanRead(const std::string& filename) override;
  void ImportActors(vtkRenderer* renderer) override;
  // ... other required methods
};
```

### Build System Notes

**Key CMake options:**
- Module toggles: `F3D_MODULE_*` (UI, raytracing, EXR, WebP)
- Plugin toggles: `F3D_PLUGIN_BUILD_*` (per-plugin enable/disable)  
- Binding toggles: `F3D_BINDINGS_*` (Python, Java)
- Build type: `BUILD_SHARED_LIBS`, `F3D_PLUGINS_STATIC_BUILD`

**Dependencies:**
- **Required**: CMake 3.21+, VTK 9.2.6+ (9.5.2 recommended), C++17 compiler
- **Optional**: Various format libraries (Assimp, OpenUSD, Alembic, etc.)
- **Testing**: Git LFS for test data

### Continuous Integration

F3D uses extensive CI across multiple platforms:

**CI Commands in PRs:**
- `\ci fast`: Quick style checks and basic Linux build
- `\ci main`: Cross-platform CI with coverage
- `\ci full`: Complete CI required before merge

**Coverage expectations:** New code should include corresponding tests to maintain coverage levels.

### Performance Considerations

- F3D emphasizes fast loading and rendering
- Plugin system allows optional dependencies to keep core lightweight
- Static plugin builds reduce startup time but increase binary size
- VTK backend provides high-performance rendering pipeline

### File Format Support

Understanding which plugin handles which formats is crucial for development:

- **Always available**: STL, PLY, OBJ, VTK formats, images (PNG, JPEG, etc.)
- **Common optional**: FBX/DAE (Assimp), USD, STEP/IGES (OpenCASCADE)
- **Scientific**: ExodusII, NetCDF, VTKHDF (HDF plugin, enabled by default)
- **Specialized**: Alembic, Draco, OpenVDB

When adding support for new formats, consider which plugin is most appropriate or if a new plugin is needed.