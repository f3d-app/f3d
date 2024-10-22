#include "F3DColoringInfoHelper.h"

#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <set>
#include <cassert>

//----------------------------------------------------------------------------
void F3DColoringInfoHelper::ClearColoringInfo()
{
  this->PointDataColoringInfo.clear();
  this->CellDataColoringInfo.clear();
}

//----------------------------------------------------------------------------
void F3DColoringInfoHelper::UpdateColoringInfo(vtkDataSet* dataset, bool useCellData)
{
  // XXX: This assumes importer do not import actors with an empty input
  assert(dataset);

  // Recover all possible names
  std::set<std::string> arrayNames;

  vtkDataSetAttributes* attr = useCellData
    ? static_cast<vtkDataSetAttributes*>(dataset->GetCellData())
    : static_cast<vtkDataSetAttributes*>(dataset->GetPointData());

  for (int i = 0; i < attr->GetNumberOfArrays(); i++)
  {
    vtkDataArray* array = attr->GetArray(i);
    if (array && array->GetName())
    {
      arrayNames.insert(array->GetName());
    }
  }

  auto& data = useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;

  for (const std::string& arrayName : arrayNames)
  {
    // Recover/Create a coloring info
    F3DColoringInfoHelper::ColoringInfo& info = data[arrayName];
    info.Name = arrayName;

    vtkDataArray* array = useCellData ? dataset->GetCellData()->GetArray(arrayName.c_str())
      : dataset->GetPointData()->GetArray(arrayName.c_str());
    if (array)
    {
      info.MaximumNumberOfComponents =
        std::max(info.MaximumNumberOfComponents, array->GetNumberOfComponents());

      // Set ranges
      // XXX this does not take animation into account
      std::array<double, 2> range;
      array->GetRange(range.data(), -1);
      info.MagnitudeRange[0] = std::min(info.MagnitudeRange[0], range[0]);
      info.MagnitudeRange[1] = std::max(info.MagnitudeRange[1], range[1]);

      for (size_t i = 0; i < static_cast<size_t>(array->GetNumberOfComponents()); i++)
      {
        array->GetRange(range.data(), static_cast<int>(i));
        if (i < info.ComponentRanges.size())
        {
          info.ComponentRanges[i][0] = std::min(info.ComponentRanges[i][0], range[0]);
          info.ComponentRanges[i][1] = std::max(info.ComponentRanges[i][1], range[1]);
        }
        else
        {
          info.ComponentRanges.emplace_back(range);
        }
      }

      // Set component names
      if (array->HasAComponentName())
      {
        for (size_t i = 0; i < static_cast<size_t>(array->GetNumberOfComponents()); i++)
        {
          const char* compName = array->GetComponentName(i);
          if (i < info.ComponentNames.size())
          {
            if (compName && info.ComponentNames[i] != std::string(compName))
            {
              // set non-coherent component names to empty string
              info.ComponentNames[i] = "";
            }
          }
          else
          {
            // Add components names to the back of the component names vector
            info.ComponentNames.emplace_back(compName ? compName : "");
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
void F3DColoringInfoHelper::FinalizeColoringInfo(bool useCellData)
{
  auto& names = useCellData ? this->CellDataArrayNames : this->PointDataArrayNames;
  names.clear();

  auto& data = useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  int index = 0;
  for (auto& [name, info] : data)
  {
    info.Index = index;
    names.emplace_back(name);
    index++;
  }
}

//----------------------------------------------------------------------------
bool F3DColoringInfoHelper::GetInfoForColoring(
  bool useCellData, int index, F3DColoringInfoHelper::ColoringInfo& info)
{
  auto& data =
    useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  auto& names =
    useCellData ? this->CellDataArrayNames : this->PointDataArrayNames;

  if (index < 0 || index >= static_cast<int>(data.size()))
  {
    return false;
  }

  info = data[names[index]];
  return true;
}

//----------------------------------------------------------------------------
int F3DColoringInfoHelper::GetNumberOfIndexesForColoring(bool useCellData)
{
  auto& data =
    useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  return static_cast<int>(data.size());
}

//----------------------------------------------------------------------------
int F3DColoringInfoHelper::FindIndexForColoring(bool useCellData, const std::string& arrayName)
{
  auto& data =
    useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;

  auto it = data.find(arrayName);
  if (it != data.end())
  {
    return it->second.Index;
  }
  return -1;
}
