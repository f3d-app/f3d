void applyCustomReader(vtkAlgorithm* reader, const std::string&) const override
{
// STLReader needs https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7012
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20200616)
  vtkSTLReader* stlReader = vtkSTLReader::SafeDownCast(reader);
  stlReader->MergingOff();
#else
  (void)reader;
#endif
}
