void applyCustomReader([[maybe_unused]] vtkAlgorithm* algo, const std::string&, [[maybe_unused]] vtkResourceStream* stream) const override
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260106)
  vtkXMLGenericDataObjectReader* doReader = vtkXMLGenericDataObjectReader::SafeDownCast(algo);
  if (stream)
  {
    doReader->ReadFromInputStreamOn();
  }
#endif
}
