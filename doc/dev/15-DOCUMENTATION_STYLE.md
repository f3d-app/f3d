# Documentation style guide

Use this guide when writing or reviewing F3D documentation.

## Assumptions

F3D readers may know 3D formats, rendering, C++, Python, CMake, or command-line tools. Do not assume they already know F3D's option names, plugin model, configuration search paths, build presets, release packages, or libf3d API shape.

## Voice and grammar

Use direct, active sentences.

- Use "you" for the reader.
- Use imperative mood for instructions: "Run", "Set", "Open", "Build".
- Use present tense for behavior: "This creates", not "This will create".
- Avoid hedging unless behavior varies by platform, package, dependency version, or build option.
- Prefer short sentences over nested clauses.

### Avoid filler

- Use "to", not "in order to".
- Use "use", not "utilize".
- Use "start", not "initiate".
- Use "end" or "quit", not "terminate", unless naming a signal or process state.
- Avoid "simply", "just", "basically", "obviously", and "easy" in instructions.

### Examples

Bad:

> In order to visualize your model, simply execute the following command, which allows F3D to load the respective file from your local machine.

Good:

> Open a model:
>
> ```bash
> f3d /path/to/model.glb
> ```

Bad:

> The user should set the configuration file path.

Good:

> Set the configuration file path:
>
> ```bash
> f3d --config=/path/to/config.json /path/to/model.glb
> ```

## Headings

Use sentence case for headings.

Good:

- `Build F3D on Linux`
- `Run the test suite`
- `Configure plugin loading`
- `Open files from the command line`

Avoid vague headings when a task heading is possible:

- Prefer `Build F3D on Linux` over `Linux`.
- Prefer `Run rendering tests` over `Rendering`.
- Prefer `Use Python bindings` over `Python`.

## Terminology

Use the same term every time. Repetition is clearer than varied synonyms in technical documentation.

### Preferred terms

| Use                 | Avoid or use only in context                              |
| ------------------- | --------------------------------------------------------- |
| F3D                 | the viewer, the app, the software, when ambiguity matters |
| `f3d` command       | F3D executable, CLI binary                                |
| libf3d              | the libf3d, LibF3D                                        |
| command-line option | CLI option, flag, parameter, unless discussing parsing    |
| configuration file  | config file, except in filenames or option names          |
| plugin              | reader module, extension, unless explaining internals     |
| reader              | importer, loader, unless the code/API uses those names    |
| file format         | file type, when referring to the supported formats table  |
| rendering backend   | OpenGL backend, when referring to `--rendering-backend`   |
| screenshot          | render capture, image dump                                |

### Product and technology names

- F3D
- f3d-app
- GitHub
- Discord
- C, C++, Java, JavaScript, Python
- WebAssembly
- Emscripten
- CMake
- VTK
- OpenGL
- Open CASCADE, OCCT
- OpenUSD
- OpenVDB
- OSPRay
- Assimp
- Draco
- glTF
- HDRI
- ImGui
- macOS, Windows, Linux, Android
- npm

Use `libf3d` when referring to the library as a product or API. Use `f3d::` only for C++ symbols.

## Commands and examples

Try to make every command copy-pasteable.

Use realistic placeholders:

- `/path/to/model.glb`
- `/path/to/output.png`
- `/path/to/build`
- `/path/to/f3d`
- `<version>`
- `<plugin-name>`

Do not use a developer's home directory, local checkout path, or release-specific filename unless the release version matters.

Bad:

```bash
tar -xzvf f3d-1.3.0-Linux.tar.gz
/home/user/dev/f3d/build/bin/f3d model.glb
```

Good:

```bash
tar -xzvf f3d-<version>-Linux.tar.gz
/path/to/f3d /path/to/model.glb
```

For multi-step procedures, use numbered lists. Put one action in each step.

Good:

1. Configure the build.
2. Build F3D.
3. Run the test.

Avoid packing several actions into one sentence.

## Code examples

Show one concept per example.

Prefer short examples that compile or run. Avoid large examples with unrelated setup. Link to [examples/](../../examples/) for complete applications.

Comments should explain intent that code cannot show on its own. Do not comment every line.

Bad:

```cpp
// Create the engine variable
f3d::engine eng = f3d::engine::create();

// Add the file to the scene
eng.getScene().add("path/to/file.ext");
```

Good:

```cpp
f3d::engine eng = f3d::engine::create();
eng.getScene().add("path/to/file.ext");
eng.getInteractor().start();
```

### C++

- Include the headers required by the snippet.
- Use API names from the public headers.
- Prefer paths like `"path/to/file.ext"` only when the exact format does not matter.
- Use real extensions, such as `.glb`, `.stl`, `.vtu`, or `.png`, when behavior depends on the format.

### Python, Java, JavaScript, and C

- Match the language's canonical capitalization in prose.
- Keep package import/setup instructions near the example.
- State build options required for the binding, such as `F3D_BINDINGS_PYTHON`.

## UI language

Match visible UI text, shortcuts, and command names exactly.

- Use `<kbd>H</kbd>`, `<kbd>Ctrl</kbd>+<kbd>Q</kbd>`, and similar markup for keys.
- Use exact command names such as `take_screenshot`, `increase`, and `set`.
- Use exact option names such as `--animation-progress` and `render.light.intensity`.
- Keep UI labels in the same casing as the application displays them.

## Links

Use descriptive link text.

Bad:

> See [here](../user/03-OPTIONS.md).

Good:

> See the [command-line options reference](../user/03-OPTIONS.md).

Before submitting a documentation change, check that relative links and anchors resolve. This matters especially when renumbering files or headings under `doc/dev`, `doc/user`, and `doc/libf3d`.

## Images and comparisons

Use images when they help readers see a rendering effect, interaction mode, or visual option.

For option illustrations:

- Reuse the existing `#### compare` pattern in [doc/user/03-OPTIONS.md](../user/03-OPTIONS.md).
- Generate images through [application/testing/tests.documentation.cmake](../../application/testing/tests.documentation.cmake) when possible.
- Store option images in [doc/user/images/](../user/images/).

For screenshots:

- Prefer repository-controlled or website-controlled assets.
- Avoid opaque `user-images.githubusercontent.com` links for long-lived documentation.
- Use stable alt text when an image is not purely decorative.

## AI-assisted writing

F3D has a dedicated [AI_POLICY.md](../../AI_POLICY.md). Follow it for documentation as well as code.

When AI generates or substantially rewrites documentation, disclose that use in the pull request. When AI only helps find typos, grammar issues, or phrasing alternatives and you make the final edits yourself, follow the policy's disclosure guidance.

## Platform notes

Put platform-specific instructions under clear headings:

- `Linux`
- `Windows`
- `macOS`
- `Android`
- `WebAssembly`

When behavior differs by platform, state the difference before the command.

Bad:

> Install the package and run F3D. On macOS, the application may be blocked.

Good:

> On macOS, the first launch may be blocked by Gatekeeper. See the troubleshooting page before running F3D.
