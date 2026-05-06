# CodeQL Build Modes for Compiled Languages

Detailed reference for how CodeQL handles compiled language analysis, including build modes, autobuild behavior, runner requirements, and hardware specifications.

## Build Modes Overview

| Mode | Description | When to Use |
|---|---|---|
| `none` | Analyze source without building. Dependencies inferred heuristically. | Default setup; quick scans |
| `autobuild` | Automatically detect and run the build system. | When `none` produces inaccurate results; when Kotlin code is present |
| `manual` | User provides explicit build commands. | Complex build systems; autobuild failures |

## C/C++

### Supported Build Modes
`none`, `autobuild`, `manual`

**Default setup mode:** `none`

### No Build (`none`)
- Infers compilation units through source file extensions
- Compilation flags and include paths inferred by inspecting the codebase
- May be less accurate for code with custom macros/defines or many external dependencies

**Improving accuracy:**
- Place custom macros/defines in header files
- Ensure external dependencies (headers) are available in system include directories
- Run extraction on the target platform

### Autobuild

**Windows autodetection:** `MSBuild.exe` on `.sln`/`.vcxproj`, then build scripts (`build.bat`, `build.cmd`, `build.exe`)

**Linux/macOS autodetection:** Looks for build system in root, then subdirectories

**Supported build systems:** MSBuild, Autoconf, Make, CMake, qmake, Meson, Waf, SCons, Linux Kbuild, build scripts

### Runner Requirements (C/C++)
- **Ubuntu:** `gcc` compiler; build tools: `msbuild`, `make`, `cmake`, `bazel`; utilities: `python`, `perl`, `lex`, `yacc`
- **Auto-install:** Set `CODEQL_EXTRACTOR_CPP_AUTOINSTALL_DEPENDENCIES=true` (enabled by default on GitHub-hosted; requires Ubuntu with passwordless `sudo apt-get`)
- **Windows:** `powershell.exe` in PATH

## C\#

### Supported Build Modes
`none`, `autobuild`, `manual`

**Default setup mode:** `none`

### No Build (`none`)
- Restores dependencies using heuristics from: `*.csproj`, `*.sln`, `nuget.config`, `packages.config`, `global.json`, `project.assets.json`
- Generates additional source files: Global `using` directives, ASP.NET Core `.cshtml` → `.cs` conversion

### Injected Compiler Flags (Manual Builds)

| Flag | Purpose |
|---|---|
| `/p:MvcBuildViews=true` | Precompile ASP.NET MVC views |
| `/p:UseSharedCompilation=false` | Disable shared compilation server |
| `/p:EmitCompilerGeneratedFiles=true` | Write generated source files to disk (may cause issues with `.sqlproj`) |

### Runner Requirements (C#)
- **.NET Core:** .NET SDK
- **.NET Framework (Windows):** Microsoft Build Tools + NuGet CLI
- **.NET Framework (Linux/macOS):** Mono Runtime (`mono`, `msbuild`, `nuget`)

## Go

### Supported Build Modes
`autobuild`, `manual` (no `none` mode)

**Default setup mode:** `autobuild`

### Autobuild
1. Invoke `make`, `ninja`, `./build`, or `./build.sh` until one succeeds and `go list ./...` works
2. Otherwise look for `go.mod`, `Gopkg.toml`, or `glide.yaml`

### Extractor Options

| Environment Variable | Default | Description |
|---|---|---|
| `CODEQL_EXTRACTOR_GO_OPTION_EXTRACT_TESTS` | `false` | Include `_test.go` files |
| `CODEQL_EXTRACTOR_GO_OPTION_EXTRACT_VENDOR_DIRS` | `false` | Include `vendor/` directories |

## Java/Kotlin

### Supported Build Modes
- **Java:** `none`, `autobuild`, `manual`
- **Kotlin:** `autobuild`, `manual` (no `none` mode)

**Default setup mode:**
- Java only: `none`
- Kotlin or Java+Kotlin: `autobuild`

> If Kotlin code is added to a repo using `none` mode, disable and re-enable default setup to switch to `autobuild`.

### Autobuild
Searches for Gradle, Maven, Ant build files; Gradle preferred over Maven.

### Runner Requirements (Java)
- JDK (appropriate version for the project)
- Gradle and/or Maven

## Rust

### Supported Build Modes
`none`, `autobuild`, `manual`

**Default setup mode:** `none`

## Swift

### Supported Build Modes
`autobuild`, `manual` (no `none` mode)

**Default setup mode:** `autobuild`

**Runner requirement:** macOS runners only. Not supported on Actions Runner Controller (ARC).

## Multi-Language Matrix Examples

### Mixed Build Modes

```yaml
strategy:
  fail-fast: false
  matrix:
    include:
      - language: c-cpp
        build-mode: manual
      - language: csharp
        build-mode: autobuild
      - language: java-kotlin
        build-mode: none
```

### Conditional Manual Build Steps

```yaml
steps:
  - name: Initialize CodeQL
    uses: github/codeql-action/init@v4
    with:
      languages: ${{ matrix.language }}
      build-mode: ${{ matrix.build-mode }}

  - if: matrix.build-mode == 'manual'
    name: Build C/C++ code
    run: |
      make bootstrap
      make release

  - name: Perform CodeQL Analysis
    uses: github/codeql-action/analyze@v4
    with:
      category: "/language:${{ matrix.language }}"
```

## Hardware Requirements

### Recommended Specifications (Self-Hosted Runners)

| Codebase Size | Lines of Code | RAM | CPU Cores | Disk |
|---|---|---|---|---|
| Small | < 100K | 8 GB+ | 2 | SSD, ≥14 GB |
| Medium | 100K – 1M | 16 GB+ | 4–8 | SSD, ≥14 GB |
| Large | > 1M | 64 GB+ | 8 | SSD, ≥14 GB |

### Performance Tips
- Use SSD storage for all codebase sizes
- Use `--threads=0` to use all available CPU cores
- Enable dependency caching to reduce analysis time
- Consider `none` build mode where accuracy is acceptable — significantly faster than `autobuild`

## Dependency Caching

```yaml
- uses: github/codeql-action/init@v4
  with:
    languages: java-kotlin
    dependency-caching: true
```

| Value | Behavior |
|---|---|
| `false` / `none` / `off` | Disabled (default for advanced setup) |
| `restore` | Restore existing caches only |
| `store` | Store new caches only |
| `true` / `full` / `on` | Restore and store caches |
