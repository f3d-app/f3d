#include <vtkCellData.h>
#include <vtkDataAssembly.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>

#include "vtkF3DFCStdReader.h"

#include <array>
#include <cmath>
#include <iostream>
#include <set>

namespace
{
constexpr double BOUNDS_TOLERANCE = 0.1;

vtkSmartPointer<vtkPartitionedDataSetCollection> ReadFile(const std::string& filename)
{
  vtkNew<vtkF3DFCStdReader> reader;
  reader->SetFileName(filename);
  reader->Update();
  reader->Print(std::cout);
  return reader->GetOutput();
}

bool CheckPartitionCount(
  vtkPartitionedDataSetCollection* pdc, unsigned int expected, const std::string& context)
{
  if (pdc->GetNumberOfPartitionedDataSets() != expected)
  {
    std::cerr << context << ": expected " << expected << " partitions, got "
              << pdc->GetNumberOfPartitionedDataSets() << "\n";
    return false;
  }
  return true;
}

bool CheckBounds(
  vtkDataSet* dataset, const std::array<double, 6>& expected, const std::string& context)
{
  double bounds[6];
  dataset->GetBounds(bounds);
  for (int i = 0; i < 6; i++)
  {
    if (std::abs(bounds[i] - expected[i]) > BOUNDS_TOLERANCE)
    {
      std::cerr << context << ": unexpected bounds [" << bounds[0] << ", " << bounds[1] << ", "
                << bounds[2] << ", " << bounds[3] << ", " << bounds[4] << ", " << bounds[5]
                << "]\n";
      return false;
    }
  }
  return true;
}
}

int TestF3DFCStdReader(int vtkNotUsed(argc), char* argv[])
{
  const std::string data = std::string(argv[1]) + "data";
  bool ret = true;

  {
    // a red 10x5x3 box and a hidden box that must not be read
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc =
      ReadFile(data + "/colored_visibility.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "colored_visibility");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      vtkDataSet* dataset = pdc->GetPartition(0, 0);
      ret &= CheckBounds(dataset, { 0., 10., 0., 5., 0., 3. }, "colored_visibility");
      vtkUnsignedCharArray* colors =
        vtkUnsignedCharArray::SafeDownCast(dataset->GetCellData()->GetScalars());
      if (!colors || colors->GetTypedComponent(0, 0) != 204 ||
        colors->GetTypedComponent(0, 1) != 26)
      {
        std::cerr << "colored_visibility: unexpected colors\n";
        ret = false;
      }
    }
  }

  {
    // a box with a different color on each face
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/face_colors.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "face_colors");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      vtkUnsignedCharArray* colors =
        vtkUnsignedCharArray::SafeDownCast(pdc->GetPartition(0, 0)->GetCellData()->GetScalars());
      std::set<std::array<unsigned char, 3>> uniqueColors;
      for (vtkIdType i = 0; colors && i < colors->GetNumberOfTuples(); i++)
      {
        uniqueColors.insert({ colors->GetTypedComponent(i, 0), colors->GetTypedComponent(i, 1),
          colors->GetTypedComponent(i, 2) });
      }
      if (uniqueColors.size() != 6)
      {
        std::cerr << "face_colors: expected 6 unique colors, got " << uniqueColors.size() << "\n";
        ret = false;
      }
    }
  }

  {
    // a hidden 2x8x2 box placed and rotated, referenced by a link placed at
    // (0, 20, 0). LinkTransform is disabled so the source placement must be
    // discarded: the box must be axis aligned at the link placement.
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/link.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "link");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      ret &= CheckBounds(pdc->GetPartition(0, 0), { 0., 2., 20., 28., 0., 2. }, "link");
    }
  }

  {
    // a 10x10x10 box placed at (3, 0, 0) inside an App::Part placed at
    // (0, 0, 10) and rotated 90 degrees around X. The container placement is
    // not baked in the stored BREP and must be composed by the reader.
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/part_container.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "part_container");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      ret &=
        CheckBounds(pdc->GetPartition(0, 0), { 3., 13., -10., 0., 10., 20. }, "part_container");
    }
  }

  {
    // saved without the FreeCAD GUI: no GuiDocument.xml, a box and a link to
    // it, both visible
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/headless.FCStd");
    ret &= CheckPartitionCount(pdc, 2, "headless");
  }

  {
    // same as link.FCStd but with LinkTransform enabled: the source placement
    // (translation and 30 degrees rotation) must be composed with the link one
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/link_transform.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "link_transform");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      ret &=
        CheckBounds(pdc->GetPartition(0, 0), { 1., 6.732, 20., 27.928, 0., 2. }, "link_transform");
    }
  }

  {
    // a link to an object from another document: skipped with a warning,
    // only the local box remains
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/link_external.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "link_external");
  }

  {
    // colored_visibility.FCStd without the App side Visibility properties,
    // like files written before FreeCAD 0.19: the GuiDocument.xml fallback
    // must still hide the hidden box
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc =
      ReadFile(data + "/legacy_visibility.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "legacy_visibility");
  }

  {
    // valid archive but garbage BREP entries: no geometry, no crash
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/corrupt_brp.FCStd");
    ret &= CheckPartitionCount(pdc, 0, "corrupt_brp");
  }

  {
    // a Draft link array: 3 instances of a hidden 2x12x3 box every 10 units.
    // The array does not store any shape, the base one is instanced at the
    // stored placements.
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/link_array.FCStd");
    ret &= CheckPartitionCount(pdc, 1, "link_array");
    if (pdc->GetNumberOfPartitionedDataSets() == 1)
    {
      ret &= CheckBounds(pdc->GetPartition(0, 0), { 0., 22., 0., 12., 0., 3. }, "link_array");
    }
  }

  {
    // a box and a Draft rectangle displayed as wireframe laying on it: the
    // rectangle face must not be rendered, only its edges when reading wires
    {
      vtkNew<vtkF3DFCStdReader> reader;
      reader->SetFileName(data + "/draft_wireframe.FCStd");
      reader->Update();
      ret &= CheckPartitionCount(reader->GetOutput(), 1, "draft_wireframe");
    }
    {
      vtkNew<vtkF3DFCStdReader> reader;
      reader->SetFileName(data + "/draft_wireframe.FCStd");
      reader->ReadWireOn();
      reader->Update();
      ret &= CheckPartitionCount(reader->GetOutput(), 2, "draft_wireframe (wires)");
    }
  }

  {
    // an opaque box, a box with 60% object transparency and a box with a 80%
    // transparent face stored with the pre-1.1 packed color semantics where
    // the last byte is a transparency, not an alpha
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/transparency.FCStd");
    ret &= CheckPartitionCount(pdc, 3, "transparency");
    if (pdc->GetNumberOfPartitionedDataSets() == 3)
    {
      const std::array<unsigned char, 3> expectedAlpha = { 255, 102, 51 };
      for (unsigned int part = 0; part < 3; part++)
      {
        vtkUnsignedCharArray* colors = vtkUnsignedCharArray::SafeDownCast(
          pdc->GetPartition(part, 0)->GetCellData()->GetScalars());
        if (!colors || colors->GetNumberOfComponents() != 4)
        {
          std::cerr << "transparency: unexpected colors for partition " << part << "\n";
          ret = false;
          continue;
        }
        // the third box is opaque except one face: look for the minimum alpha
        unsigned char minAlpha = 255;
        for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); i++)
        {
          minAlpha = std::min(minAlpha, colors->GetTypedComponent(i, 3));
        }
        if (minAlpha != expectedAlpha[part])
        {
          std::cerr << "transparency: unexpected alpha " << static_cast<int>(minAlpha)
                    << " for partition " << part << "\n";
          ret = false;
        }
      }
    }
  }

  {
    // a file saved with FreeCAD 1.1, which stores colors differently from
    // older versions: two parts with curved surfaces, one with a single
    // color, one with a color per face
    vtkSmartPointer<vtkPartitionedDataSetCollection> pdc = ReadFile(data + "/bracket_1x.FCStd");
    ret &= CheckPartitionCount(pdc, 2, "bracket_1x");
    if (pdc->GetNumberOfPartitionedDataSets() == 2)
    {
      // placed at (0, 0, 5): 8 units base and 20 units boss
      double bounds[6];
      pdc->GetPartition(0, 0)->GetBounds(bounds);
      if (std::abs(bounds[4] - 5.) > BOUNDS_TOLERANCE ||
        std::abs(bounds[5] - 25.) > BOUNDS_TOLERANCE)
      {
        std::cerr << "bracket_1x: unexpected bracket bounds [" << bounds[4] << ", " << bounds[5]
                  << "]\n";
        ret = false;
      }
      const std::array<size_t, 2> expectedColors = { 1, 4 };
      for (unsigned int part = 0; part < 2; part++)
      {
        vtkUnsignedCharArray* colors = vtkUnsignedCharArray::SafeDownCast(
          pdc->GetPartition(part, 0)->GetCellData()->GetScalars());
        std::set<std::array<unsigned char, 3>> uniqueColors;
        for (vtkIdType i = 0; colors && i < colors->GetNumberOfTuples(); i++)
        {
          uniqueColors.insert({ colors->GetTypedComponent(i, 0), colors->GetTypedComponent(i, 1),
            colors->GetTypedComponent(i, 2) });
        }
        if (part == 1 && colors)
        {
          // the third tube material has a 0.5 transparency in the 1.x
          // material list, at least one cell must have a half alpha
          bool hasHalfAlpha = false;
          for (vtkIdType i = 0; i < colors->GetNumberOfTuples(); i++)
          {
            hasHalfAlpha |= colors->GetTypedComponent(i, 3) == 128;
          }
          if (!hasHalfAlpha)
          {
            std::cerr << "bracket_1x: expected half transparent cells on the tube\n";
            ret = false;
          }
        }
        if (uniqueColors.size() != expectedColors[part])
        {
          std::cerr << "bracket_1x: partition " << part << ": expected " << expectedColors[part]
                    << " unique colors, got " << uniqueColors.size() << "\n";
          ret = false;
        }
      }
    }
  }

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
