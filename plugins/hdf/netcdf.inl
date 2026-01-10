void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream*) const override
{
  vtkNetCDFReader* ncReader = vtkNetCDFReader::SafeDownCast(algo);
  ncReader->UpdateInformation();
  
  int numArrays = ncReader->GetNumberOfVariableArrays();
  for (int i = 0; i < numArrays; i++)
  {
    const char* arrayName = ncReader->GetVariableArrayName(i);
    if (arrayName)
    {
      ncReader->SetVariableArrayStatus(arrayName, 1);
    }
  }
}
