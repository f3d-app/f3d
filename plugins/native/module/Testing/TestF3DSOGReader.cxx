#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkExecutive.h>
#include <vtkFileResourceStream.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkTestUtilities.h>
#include <vtkUnsignedCharArray.h>

#include "vtkF3DSOGReader.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
bool Near(const char* label, double value, double expected)
{
  if (std::abs(value - expected) > 1e-5)
  {
    std::cerr << label << ": expected " << expected << " got " << value << "\n";
    return false;
  }
  return true;
}

void RecordError(vtkObject*, unsigned long, void* clientData, void* callData)
{
  if (callData)
  {
    static_cast<std::vector<std::string>*>(clientData)->emplace_back(static_cast<char*>(callData));
  }
}
}

int TestF3DSOGReader(int vtkNotUsed(argc), char* argv[])
{
  if (vtkF3DSOGReader::CanReadFile(nullptr))
  {
    std::cerr << "Unexpected CanReadFile success with nullptr\n";
    return EXIT_FAILURE;
  }

  const std::string data = std::string(argv[1]) + "data/";

  vtkNew<vtkFileResourceStream> stream;
  if (!stream->Open((data + "tiny.sog").c_str()))
  {
    std::cerr << "Cannot open tiny.sog\n";
    return EXIT_FAILURE;
  }

  // CanReadFile has to leave the cursor where it found it.
  stream->Seek(4, vtkResourceStream::SeekDirection::Begin);
  const vtkTypeInt64 position = stream->Tell();
  if (!vtkF3DSOGReader::CanReadFile(stream) || stream->Tell() != position)
  {
    std::cerr << "Unexpected CanReadFile failure for tiny.sog\n";
    return EXIT_FAILURE;
  }

  vtkNew<vtkF3DSOGReader> reader;
  reader->SetStream(stream);
  reader->Update();

  vtkPolyData* output = reader->GetOutput();
  vtkPoints* points = output->GetPoints();
  // 2x2 images, count 3. Gaussian 2 is the first pixel of the second row.
  constexpr vtkIdType gaussian = 2;
  if (!points || points->GetDataType() != VTK_FLOAT || points->GetNumberOfPoints() != 3 ||
    !points->GetData() || !points->GetData()->GetName() ||
    std::string(points->GetData()->GetName()) != "position")
  {
    std::cerr << "Unexpected position points\n";
    return EXIT_FAILURE;
  }

  // mins [0, -ln(2), 0], maxs [ln(2), 0, 1]
  // low bytes (255, 0, 0), high bytes (255, 0, 128)
  // q = (65535, 0, 32768) -> (1, -1, exp(32768/65535) - 1)
  double point[3];
  points->GetPoint(gaussian, point);
  if (!::Near("position.x", point[0], 1.0) || !::Near("position.y", point[1], -1.0) ||
    !::Near("position.z", point[2], 0.6487338542938232))
  {
    return EXIT_FAILURE;
  }

  vtkUnsignedCharArray* color =
    vtkUnsignedCharArray::SafeDownCast(output->GetPointData()->GetScalars());
  if (!color || !color->GetName() || std::string(color->GetName()) != "color" ||
    color->GetNumberOfComponents() != 4 || color->GetNumberOfTuples() != 3)
  {
    std::cerr << "Unexpected color array\n";
    return EXIT_FAILURE;
  }

  // sh0 codebook[0] = 0, codebook[4] = 1, pixel (4, 0, 4, 200)
  // 0.5 + 0 * SH_C0 = 0.5 -> 127.5, truncated to 127
  // 0.5 + 1 * 0.28209479177387814 = 0.7820947917738781 -> 199.434..., truncated to 199
  // version 2 opacity is the alpha byte
  const unsigned char expectedColor[4] = { 199, 127, 199, 200 };
  for (int component = 0; component < 4; ++component)
  {
    const unsigned char value = color->GetTypedComponent(gaussian, component);
    if (value != expectedColor[component])
    {
      std::cerr << "color[" << component << "]: expected "
                << static_cast<int>(expectedColor[component]) << " got " << static_cast<int>(value)
                << "\n";
      return EXIT_FAILURE;
    }
  }

  vtkFloatArray* scale = vtkFloatArray::SafeDownCast(output->GetPointData()->GetArray("scale"));
  if (!scale || scale->GetNumberOfComponents() != 3 || scale->GetNumberOfTuples() != 3)
  {
    std::cerr << "Unexpected scale array\n";
    return EXIT_FAILURE;
  }

  // codebook[0] = 0, [1] = ln(2), [2] = -ln(2), pixel (1, 2, 0) -> (2, 0.5, 1)
  if (!::Near("scale.x", scale->GetTypedComponent(gaussian, 0), 2.0) ||
    !::Near("scale.y", scale->GetTypedComponent(gaussian, 1), 0.5) ||
    !::Near("scale.z", scale->GetTypedComponent(gaussian, 2), 1.0))
  {
    return EXIT_FAILURE;
  }

  vtkFloatArray* rotation =
    vtkFloatArray::SafeDownCast(output->GetPointData()->GetArray("rotation"));
  if (!rotation || rotation->GetNumberOfComponents() != 4 || rotation->GetNumberOfTuples() != 3)
  {
    std::cerr << "Unexpected rotation array\n";
    return EXIT_FAILURE;
  }

  // bytes (64, 128, 192, 253): mode 1, so x is the omitted component.
  // toComp(c) = (c/255 - 0.5) * 2/sqrt(2)
  // w = toComp(64), x = sqrt(1 - sum of squares), y = toComp(128), z = toComp(192)
  const double expectedRotation[4] = { -0.35216692090034485, 0.8648770451545715,
    0.0027729677967727184, 0.35771283507347107 };
  for (int component = 0; component < 4; ++component)
  {
    if (!::Near("rotation", rotation->GetTypedComponent(gaussian, component),
          expectedRotation[component]))
    {
      return EXIT_FAILURE;
    }
  }

  if (!stream->Open((data + "cube_gears.3mf").c_str()))
  {
    std::cerr << "Cannot open cube_gears.3mf\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DSOGReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success for a zip without meta.json\n";
    return EXIT_FAILURE;
  }

  if (!stream->Open((data + "f3d.vtp").c_str()))
  {
    std::cerr << "Cannot open f3d.vtp\n";
    return EXIT_FAILURE;
  }

  if (vtkF3DSOGReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile success for a file that is not a zip\n";
    return EXIT_FAILURE;
  }

  if (!stream->Open((data + "version3.sog").c_str()))
  {
    std::cerr << "Cannot open version3.sog\n";
    return EXIT_FAILURE;
  }

  if (!vtkF3DSOGReader::CanReadFile(stream))
  {
    std::cerr << "Unexpected CanReadFile failure for version 3\n";
    return EXIT_FAILURE;
  }

  std::vector<std::string> errors;
  vtkNew<vtkCallbackCommand> recordError;
  recordError->SetClientData(&errors);
  recordError->SetCallback(::RecordError);

  vtkNew<vtkCallbackCommand> ignoreError;
  ignoreError->SetCallback([](vtkObject*, unsigned long, void*, void*) {});

  vtkNew<vtkF3DSOGReader> versionReader;
  versionReader->SetStream(stream);
  versionReader->AddObserver(vtkCommand::ErrorEvent, recordError);
  versionReader->GetExecutive()->AddObserver(vtkCommand::ErrorEvent, ignoreError);
  if (versionReader->Update())
  {
    std::cerr << "Unexpected Update success for version 3\n";
    return EXIT_FAILURE;
  }

  bool reported = false;
  for (const std::string& message : errors)
  {
    if (message.find("Only version 2 is supported") != std::string::npos)
    {
      reported = true;
    }
  }
  if (!reported)
  {
    std::cerr << "Version 3 did not report that only version 2 is supported\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
