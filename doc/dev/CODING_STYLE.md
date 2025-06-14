# Coding Style

F3D use different coding styles in each component, however there are some common rules

## C++

### Common rules

Overall syntax:

- CamelCase.
- Avoid using `using` for namespaces.
- Initialize variables in header when possible.
- Local variables starts with a lower case char.
- Class members starts with a upper case char.
- Indents are two spaces.
- One space between instruction and parenthesis.
- Always add curly brace after instruction.
- Curly brace one a new line, aligned with instruction.
- Add `//----------------------------------------------------------------------------` before any method in implementation.

Example:

```cpp
//----------------------------------------------------------------------------
void class::method()
{
  if (condition)
  {
    std::vector<int> myIntVector = { 13 };
  }
}
```

Includes:

- Organized by category: `F3DApplication`, `libf3d`, `VTKExtensions`, `other deps`, `std`, `system`.
- Sorted inside category.

### F3D Application rules

- Class starts with `F3D`
- Method starts with an uppercase char.

### libf3d rules

- Class starts with a lower case char.
- Method starts with an lower case char.

### VTKExtensions rules

- Follow VTK rules
- Method starts with an uppercase char.
- Class starts with `vtkF3D` if inheriting from vtkObject.
- Class starts with `F3D` if not inheriting from vtkObject.

### Automatic formatting

Some of the rules above are enforced using clang-format thanks to a `.clang-format` file.
The continuous integration checks it whenever code is pushed to a pull request.
To fix it locally, you can use:

- single file: `clang-format -i /path/to/file.ext`
- all files: `shopt -s globstar; clang-format -i **/*.{h,cxx}`

Please note there can be small discrepancy between the CI and local run of clang-format depending
on the version in use. You may need to fix these manually.

## Python

All python code is simply formatted using [Black style](https://black.readthedocs.io/en/stable/the_black_code_style/current_style.html).

The continuous integration checks the formatting of python code using `black`.
You can fix it locally by running:

- single file: `black /path/to/file.py`
- all files: `black --include '(\.py|\.py\.in)' .`

## Markdown and others

Markdown, JavaScript, JSON, HTML and YAML files are formatted using [Prettier](https://prettier.io/docs/).

The continuous integration checks the formatting of all these files using `prettier`.
You can fix them locally by running:

- single file: `prettier -w /path/to/file.ext`
- all files: `shopt -s dotglob;shopt -s globstar; prettier -w **/*.{js,json,md,html,yml}`
