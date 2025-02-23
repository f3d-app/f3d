void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkPDALReader* pdalReader = vtkPDALReader::SafeDownCast(algo);
  pdalReader->UpdateInformation();
}
