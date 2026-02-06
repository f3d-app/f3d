void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream* stream) const override
{
  vtkPLYReader* plyReader = vtkPLYReader::SafeDownCast(algo);
  if (stream)
  {
    plyReader->ReadFromInputStreamOn();
  }
}
