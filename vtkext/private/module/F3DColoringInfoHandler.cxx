#include "F3DColoringInfoHandler.h"

#include "F3DLog.h"

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <cassert>
#include <set>

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
std::optional<F3DColoringInfoHandler::ColoringInfo> F3DColoringInfoHandler::SetCurrentColoring(
  bool enable, bool useCellData, const std::optional<std::string>& arrayName, bool quiet)
{
  this->CurrentUsingCellData = useCellData;
  auto& data =
    this->CurrentUsingCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  int nIndices = static_cast<int>(data.size());

  if (!enable)
  {
    // Not coloring
    this->CurrentColoringIter.reset();
  }
  else if (nIndices == 0)
  {
    // Trying to color but no array available
    this->CurrentColoringIter.reset();

    if (!quiet)
    {
      F3DLog::Print(F3DLog::Severity::Debug, "No array to color with");
    }
  }
  else if (!arrayName.has_value())
  {
    // Coloring with first array
    this->CurrentColoringIter = data.begin();
  }
  else
  {
    // Coloring with named array
    this->CurrentColoringIter = data.find(arrayName.value());
    if (this->CurrentColoringIter.value() == data.end())
    {
      // Could not find named array
      this->CurrentColoringIter.reset();
      if (!quiet)
      {
        F3DLog::Print(
          F3DLog::Severity::Warning, "Unknown scalar array: \"" + arrayName.value() + "\"");
      }
    }
  }
  return this->GetCurrentColoringInfo();
}

//----------------------------------------------------------------------------
std::optional<F3DColoringInfoHandler::ColoringInfo> F3DColoringInfoHandler::GetCurrentColoringInfo()
  const
{
  if (this->CurrentColoringIter.has_value())
  {
    return this->CurrentColoringIter.value()->second;
  }
  return std::nullopt;
}

//----------------------------------------------------------------------------
void F3DColoringInfoHandler::CycleColoringArray(bool cycleToNonColoring)
{
  const auto& data =
    this->CurrentUsingCellData ? this->CellDataColoringInfo : this->PointDataColoringInfo;
  if (!this->CurrentColoringIter.has_value())
  {
    if (!data.empty())
    {
      this->CurrentColoringIter = data.begin();
    }
  }
  else
  {
    if (++this->CurrentColoringIter.value() == data.end())
    {
      if (cycleToNonColoring)
      {
        this->CurrentColoringIter.reset();
      }
      else
      {
        this->CurrentColoringIter = data.begin();
      }
    }
  }
}
