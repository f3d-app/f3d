# Parsing options

When setting options from the [CLI Options](OPTIONS.md), the [commands](COMMANDS.md) or using the [libf3d options string API](../libf3d/OPTIONS.md#string-api), the values are parsed according to their type. If parsing fails, the value is not changed.

The following types are supported:

- bool: A boolean, true or false.
- int: A signed integer.
- double: A floating point number.
- ratio: A double dividend over a double divisor, stored in a double.
- string: A string of characters.
- color: A RGB color
- direction: A 3D vector representing a direction

As well as a list for bool, int, double, ratio, string, noted as

- vector\<type\>

## Bool

The following formats are supported when parsing a bool, case insensitive:

- true/false
- yes/no
- on/off
- 1/0

When formatting a bool into a string, true/false is used.

## Int

Int parsing is supported using [std::stoi](https://en.cppreference.com/w/cpp/string/basic_string/stol) and check
that the whole string is parsed.

When formatting an int into a string, [std::to_string](https://en.cppreference.com/w/cpp/string/basic_string/to_string) is used.

## Double

Double parsing is supported using [std::stod](https://en.cppreference.com/w/cpp/string/basic_string/stol) and check
that the whole string is parsed.

When formatting a double into a string, [std::ostringstream](https://en.cppreference.com/w/cpp/io/basic_ostringstream) is used
with removing the point and precision when the value is exactly an integer.

## Ratio

The following formats are supported when parsing a string into a ratio:

- percent% where percent is a double
- dividend:divisor where both are doubles
- dividend/divisor where both are doubles
- double

Percent, dividend, divisor are then parsed as double.

When formatting a ratio into a string, it is formatted as a double.

## String

String are parsed and formatted as is.

## Vectors

Vector tokens are separated by `,`, tokens are then parsed using their respective types.

When formatting a vector into a string, individual token are formatted according to their type and separated using `,`.

## Color

The following formats are supported when parsing a color, case insensitive:

- R,G,B where R, G, B are doubles >= 0
- #RRGGBB where RR, GG, BB are hexadecimal values. Shortened format #RGB is also valid.
- rgb(R, G, B) where R, G and B are integer [0, 255]
- hsl(H, S%, L%) where H is integer [0, 360], S and L are integer [0, 100]
- hsv(H, S%, V%) where H is integer [0, 360], S and V are integer [0, 100]
- hwb(H, W%, B%) where H is integer [0, 360], W and B are integer [0, 100]
- cmyk(C%, M%, Y%, K%) where C, M, Y, K are integer [0, 100]
- [color name](https://htmlpreview.github.io/?https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html)

See [W3C](https://www.w3.org/TR/css-color-3/#rgb-color) doc for more details on these formats.

When formatting a color into a string, it is formatted as `#RRGGBB` if values are multiple of 255. Otherwise, it is formatted as vector of doubles.

## Direction

The following formats are supported when parsing a string into a direction:

- [+/-][X/Y/Z]
- vector of three doubles

When formatting a direction into a string, it is formatted as a vector of doubles.
