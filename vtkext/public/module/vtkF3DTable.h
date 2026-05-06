/**
 * @class   vtkF3DTable
 * @brief   Extended vtkTable with utility methods for F3D
 */

#ifndef vtkF3DTable_h
#define vtkF3DTable_h

#include "vtkextModule.h"

#include <vtkTable.h>

class vtkDataArray;

class VTKEXT_EXPORT vtkF3DTable : public vtkTable
{
public:
  static vtkF3DTable* New();
  vtkTypeMacro(vtkF3DTable, vtkTable);

  /**
   * Add a data array as a column to this table.
   * If name is not specified, the array's name will be used.
   * @param array The data array to add as a column
   * @param name The name for the column (optional, defaults to array's name)
   */
  void AddDataArray(vtkDataArray* array, const std::string& name = "");

protected:
  vtkF3DTable();
  ~vtkF3DTable() override;

private:
  vtkF3DTable(const vtkF3DTable&) = delete;
  void operator=(const vtkF3DTable&) = delete;
};

#endif
