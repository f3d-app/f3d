void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkSTLReader* stlReader = vtkSTLReader::SafeDownCast(algo);
  stlReader->MergingOff();
}
