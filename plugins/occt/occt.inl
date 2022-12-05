void applyCustomReader(vtkAlgorithm* algo, const std::string& fileName) const override
{
  vtkF3DOCCTReader* occtReader = vtkF3DOCCTReader::SafeDownCast(algo);

  occtReader->RelativeDeflectionOn();
  occtReader->SetLinearDeflection(0.1);
  occtReader->SetAngularDeflection(0.5);
  occtReader->ReadWireOn();
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
  ext = vtksys::SystemTools::LowerCase(ext);
  using ff = vtkF3DOCCTReader::FILE_FORMAT;
  occtReader->SetFileFormat((ext == ".stp" || ext == ".step") ? ff::STEP : ff::IGES);
}
