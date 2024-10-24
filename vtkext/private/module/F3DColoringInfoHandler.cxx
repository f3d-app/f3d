#include "F3DColoringInfoHandler.h"

#include "F3DLog.h"

#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <set>
#include <cassert>

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::ClearColoringInfo()
{
  this->PointDataColoringInfo.clear();
  this->CellDataColoringInfo.clear();
}

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::UpdateColoringInfo(vtkDataSet* dataset, bool useCellData)
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
    F3DColoringInfoHandler::ColoringInfo& info = data[arrayName];
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
void F3DColoringInfoHandler::FinalizeColoringInfo(bool useCellData)
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
/*bool F3DColoringInfoHandler::GetInfoForColoring(
  bool useCellData, int index, F3DColoringInfoHandler::ColoringInfo& info)
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
int F3DColoringInfoHandler::GetNumberOfIndicesForColoring(bool useCellData)
{
  auto& data =
    useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  return static_cast<int>(data.size());
}

//----------------------------------------------------------------------------
int F3DColoringInfoHandler::FindIndexForColoring(bool useCellData, const std::string& arrayName)
{
  auto& data =
    useCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;

  auto it = data.find(arrayName);
  if (it != data.end())
  {
    return it->second.Index;
  }
  return -1;
}*/

//----------------------------------------------------------------------------
bool F3DColoringInfoHandler::SetCurrentColoring(bool enable, bool useCellData, std::optional<std::string> arrayName, ColoringInfo& info)
{
  this->CurrentUsingCellData = useCellData;
  auto& data =
    this->CurrentUsingCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  int nIndices = static_cast<int>(data.size());

  if (!enable)
  {
    // Not coloring
    this->CurrentArrayIndex = -1;
  }
  else if (nIndices == 0)
  {
    // Trying to color but no array available
    F3DLog::Print(F3DLog::Severity::Debug, "No array to color with");
    this->CurrentArrayIndex = -1;
  }
  else if (!arrayName.has_value())
  {
    // Coloring with first array
    this->CurrentArrayIndex = 0;
  }
  else
  {
    // Coloring with named array
    auto it = data.find(arrayName.value());
    if (it != data.end())
    {
      this->CurrentArrayIndex = it->second.Index;
    }
    else
    {
      // Could not find named array
      this->CurrentArrayIndex = -1;
      F3DLog::Print(F3DLog::Severity::Warning, "Unknown scalar array: \"" + arrayName.value() + "\"\n");
    }
  }
  return this->GetCurrentColoring(info);
}

//----------------------------------------------------------------------------
bool F3DColoringInfoHandler::GetCurrentColoring(ColoringInfo& info)
{
  if (this->CurrentArrayIndex != -1)
  {
    auto& data =
      this->CurrentUsingCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
    auto& names = this->CurrentUsingCellData ? this->CellDataArrayNames : this->PointDataArrayNames;
    info = data[names[this->CurrentArrayIndex]];
    return true;
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::CycleColoringArray(bool cycleToNonColoring)
{
  auto& data =
    this->CurrentUsingCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  int nIndices = static_cast<int>(data.size());
  if (nIndices <= 0)
  {
    return;
  }

  if (cycleToNonColoring)
  {
    // Cycle through arrays looping back to -1
    // -1 0 1 2 -1 0 1 2 ...
    this->CurrentArrayIndex = (this->CurrentArrayIndex + 2) % (nIndices + 1) - 1;
  }
  else
  {
    this->CurrentArrayIndex = (this->CurrentArrayIndex + 1) % nIndices;
  }
}
