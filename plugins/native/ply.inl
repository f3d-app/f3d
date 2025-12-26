void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream* stream) const override
{
  vtkPLYReader* reader = vtkPLYReader::SafeDownCast(algo);
  if (stream)
  {
    reader->ReadFromInputStreamOn();
  }
}
