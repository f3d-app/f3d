void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream*) const override
{
  vtkSTLReader* stlReader = vtkSTLReader::SafeDownCast(algo);
  stlReader->MergingOff();
}
