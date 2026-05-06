#include "vtkF3DTable.h"

#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>

#include <iostream>

int TestF3DTable(int argc, char* argv[])
{
  vtkObject::GlobalWarningDisplayOff();

  // Test basic AddDataArray functionality
  vtkNew<vtkF3DTable> table;

  // Test with a float array
  vtkNew<vtkFloatArray> floatArray;
  floatArray->SetName("FloatColumn");
  floatArray->InsertNextValue(1.0f);
  floatArray->InsertNextValue(2.0f);
  floatArray->InsertNextValue(3.0f);

  table->AddDataArray(floatArray);

  if (table->GetNumberOfColumns() != 1)
  {
    std::cerr << "Expected 1 column after adding first array, got "
              << table->GetNumberOfColumns() << std::endl;
    return EXIT_FAILURE;
  }

  vtkAbstractArray* col0 = table->GetColumn(0);
  if (!col0 || std::string(col0->GetName()) != "FloatColumn")
  {
    std::cerr << "Column 0 name mismatch or null" << std::endl;
    return EXIT_FAILURE;
  }

  // Test with custom name
  vtkNew<vtkIntArray> intArray;
  intArray->SetName("OriginalName");
  intArray->InsertNextValue(10);
  intArray->InsertNextValue(20);
  intArray->InsertNextValue(30);

  table->AddDataArray(intArray, "CustomColumn");

  if (table->GetNumberOfColumns() != 2)
  {
    std::cerr << "Expected 2 columns after adding second array, got "
              << table->GetNumberOfColumns() << std::endl;
    return EXIT_FAILURE;
  }

  vtkAbstractArray* col1 = table->GetColumn(1);
  if (!col1 || std::string(col1->GetName()) != "CustomColumn")
  {
    std::cerr << "Column 1 name should be 'CustomColumn', got "
              << (col1 ? col1->GetName() : "null") << std::endl;
    return EXIT_FAILURE;
  }

  // Note: The array's name is now "CustomColumn" after being added to the table
  // This is expected VTK behavior - the array IS the column

  // Test with auto-generated name (empty name parameter)
  vtkNew<vtkFloatArray> autoNamedArray;
  // Don't set a name, should use default
  autoNamedArray->InsertNextValue(100.0f);

  table->AddDataArray(autoNamedArray);

  if (table->GetNumberOfColumns() != 3)
  {
    std::cerr << "Expected 3 columns after adding third array, got "
              << table->GetNumberOfColumns() << std::endl;
    return EXIT_FAILURE;
  }

  vtkAbstractArray* col2 = table->GetColumn(2);
  if (!col2 || std::string(col2->GetName()).empty())
  {
    std::cerr << "Column 2 should have an auto-generated name" << std::endl;
    return EXIT_FAILURE;
  }

  // Test with null array (should warn and not crash)
  table->AddDataArray(nullptr);

  if (table->GetNumberOfColumns() != 3)
  {
    std::cerr << "Adding null array should not change column count" << std::endl;
    return EXIT_FAILURE;
  }

  // Verify data integrity
  vtkFloatArray* retrievedFloatArray = vtkFloatArray::SafeDownCast(table->GetColumn(0));
  if (!retrievedFloatArray || retrievedFloatArray->GetValue(0) != 1.0f)
  {
    std::cerr << "Data integrity check failed for float array" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
