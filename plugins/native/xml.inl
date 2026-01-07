void applyCustomReader(vtkAlgorithm* algo, const std::string&, vtkResourceStream* stream) const override
{
  vtkXMLGenericDataObjectReader* doReader = vtkXMLGenericDataObjectReader::SafeDownCast(algo);
  if (stream)
  {
    doReader->ReadFromInputStreamOn();
  }
}
