#include "F3DColoringInfoHandler.h"

#include "F3DLog.h"

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <algorithm>
#include <cassert>
#include <set>

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::ClearColoringInfo()
{
  this->ColoringInfoMap.clear();
  this->CurrentColoringIter = this->ColoringInfoMap.end();
}

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::UpdateColoringInfo(vtkDataSet* dataset)
{
  // XXX: This assumes importer do not import actors with an empty input
  assert(dataset);

  for (const bool useCellData : { false, true })
  {
    vtkDataSetAttributes* attr = useCellData
      ? static_cast<vtkDataSetAttributes*>(dataset->GetCellData())
      : static_cast<vtkDataSetAttributes*>(dataset->GetPointData());

    // Recover all possible names
    std::set<std::string> arrayNames;

    for (int i = 0; i < attr->GetNumberOfArrays(); i++)
    {
      vtkDataArray* array = attr->GetArray(i);
      if (array && array->GetName())
      {
        arrayNames.insert(array->GetName());
      }
    }

    for (const std::string& arrayName : arrayNames)
    {
      // Recover/Create a coloring info
      F3DColoringInfoHandler::ColoringInfo& info =
        this->ColoringInfoMap[{ arrayName, useCellData }];
      info.Name = arrayName;
      info.IsCellData = useCellData;

      vtkDataArray* array = attr->GetArray(arrayName.c_str());
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

  this->CurrentColoringIter = this->ColoringInfoMap.end();
}

void F3DColoringInfoHandler::SelectFirstArray(bool forceUsePointData, bool forceUseCellData)
{
  if (forceUseCellData)
  {
    this->CurrentColoringIter = std::ranges::find_if(
      this->ColoringInfoMap, [](const auto& pair) { return pair.first.second == true; });
  }
  else if (forceUsePointData)
  {
    this->CurrentColoringIter = std::ranges::find_if(
      this->ColoringInfoMap, [](const auto& pair) { return pair.first.second == false; });
  }
  else
  {
    this->CurrentColoringIter = this->ColoringInfoMap.begin();
  }
}

//----------------------------------------------------------------------------
std::optional<F3DColoringInfoHandler::ColoringInfo> F3DColoringInfoHandler::SetCurrentColoring(
  bool forceUsePointData, bool forceUseCellData, std::optional<bool>& arrayIsCellData,
  std::optional<std::string>& arrayName, bool quiet)
{
  const int nIndices = static_cast<int>(this->ColoringInfoMap.size());

  this->CurrentColoringIter = this->ColoringInfoMap.end();

  if (nIndices == 0)
  {
    // No array available
    if (!quiet)
    {
      F3DLog::Print(F3DLog::Severity::Debug, "No array to color with");
    }
  }
  else if (arrayName.has_value())
  {
    // Coloring with named array
    if (!forceUseCellData && !arrayIsCellData.value_or(false))
    {
      this->CurrentColoringIter = this->ColoringInfoMap.find({ arrayName.value(), false });
    }

    if (this->CurrentColoringIter == this->ColoringInfoMap.end() && !forceUsePointData &&
      arrayIsCellData.value_or(true))
    {
      this->CurrentColoringIter = this->ColoringInfoMap.find({ arrayName.value(), true });
    }

    if (this->CurrentColoringIter == this->ColoringInfoMap.end())
    {
      // Could not find named array
      if (!quiet)
      {
        std::string fieldDesc;
        if (forceUseCellData)
        {
          fieldDesc = " (cell data)";
        }
        else if (forceUsePointData)
        {
          fieldDesc = " (point data)";
        }
        F3DLog::Print(F3DLog::Severity::Warning,
          "Unknown scalar array: \"" + arrayName.value() + "\"" + fieldDesc);
      }

      arrayIsCellData.reset();
      arrayName.reset();
      this->SelectFirstArray(forceUsePointData, forceUseCellData);
    }
  }
  else
  {
    // Default to the first available array if no array name is provided
    this->SelectFirstArray(forceUsePointData, forceUseCellData);
  }
  return this->GetCurrentColoringInfo();
}

//----------------------------------------------------------------------------
std::optional<F3DColoringInfoHandler::ColoringInfo> F3DColoringInfoHandler::GetCurrentColoringInfo()
  const
{
  if (this->CurrentColoringIter != this->ColoringInfoMap.end())
  {
    return this->CurrentColoringIter->second;
  }
  return std::nullopt;
}

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::CycleColoringArray(bool forceUsePointData, bool forceUseCellData)
{
  if (this->CurrentColoringIter != this->ColoringInfoMap.end())
  {
    if (forceUseCellData)
    {
      this->CurrentColoringIter = std::find_if(std::next(this->CurrentColoringIter),
        this->ColoringInfoMap.cend(), [](const auto& pair) { return pair.first.second == true; });
    }
    else if (forceUsePointData)
    {
      this->CurrentColoringIter = std::find_if(std::next(this->CurrentColoringIter),
        this->ColoringInfoMap.cend(), [](const auto& pair) { return pair.first.second == false; });
    }
    else
    {
      this->CurrentColoringIter++;
    }
  }

  if (this->CurrentColoringIter == this->ColoringInfoMap.end())
  {
    this->SelectFirstArray(forceUsePointData, forceUseCellData);
  }
}
