# Coding Style

F3D use different coding styles in each component, however there are some common rules

## Common rules

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

## F3D Application rules

- Class starts with `F3D`
- Method starts with an uppercase char.

## libf3d rules

- Class starts with a lower case char.
- Method starts with an lower case char.

## VTKExtensions rules

- Follow VTK rules
- Method starts with an uppercase char.
- Class starts with `vtkF3D` if inheriting from vtkObject.
- Class starts with `F3D` if not inheriting from vtkObject.
