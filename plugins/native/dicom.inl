using reader::canRead;
bool canRead([[maybe_unused]] vtkResourceStream* stream) const override
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 6, 20260128)
  vtkNew<vtkDICOMImageReader> tmpReader;
  return tmpReader->CanReadFile(stream);
#else
  return true;
#endif
}
