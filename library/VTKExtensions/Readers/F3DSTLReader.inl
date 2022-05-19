void ApplyCustomReader(vtkAlgorithm* reader, const std::string&) const override
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200616)
  vtkSTLReader* stlReader = vtkSTLReader::SafeDownCast(reader);
  stlReader->MergingOff();
#else
  (void)reader;
#endif
}
