#include "vtkF3DTable.h"

#include <vtkDataArray.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkF3DTable);

//----------------------------------------------------------------------------
vtkF3DTable::vtkF3DTable()
{
}

//----------------------------------------------------------------------------
vtkF3DTable::~vtkF3DTable()
{
}

//----------------------------------------------------------------------------
void vtkF3DTable::AddDataArray(vtkDataArray* array, const std::string& name)
{
  if (!array)
  {
    vtkWarningMacro("Cannot add null data array to table.");
    return;
  }

  std::string columnName = name;
  if (columnName.empty())
  {
    const char* arrayName = array->GetName();
    if (!arrayName || arrayName[0] == '\0')
    {
      columnName = "Column" + std::to_string(this->GetNumberOfColumns());
    }
    else
    {
      columnName = arrayName;
    }
  }

  array->SetName(columnName.c_str());
  this->AddColumn(array);
}
