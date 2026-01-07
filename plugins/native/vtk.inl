void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream* stream) const override
{
  vtkDataSetReader* dsReader = vtkDataSetReader::SafeDownCast(algo);
  if (stream)
  {
    dsReader->ReadFromInputStreamOn();
  }
}
