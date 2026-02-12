using reader::canRead;
bool canRead(vtkResourceStream* stream) const override
{
  vtkNew<vtkDICOMImageReader> tmpReader;
  return tmpReader->CanReadFile(stream);
}
