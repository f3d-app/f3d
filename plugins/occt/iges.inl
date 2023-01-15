void applyCustomReader(vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName)) const override
{
  vtkF3DOCCTReader* occtReader = vtkF3DOCCTReader::SafeDownCast(algo);

  occtReader->RelativeDeflectionOn();
  occtReader->SetLinearDeflection(0.1);
  occtReader->SetAngularDeflection(0.5);
  occtReader->ReadWireOn();
  occtReader->SetFileFormat(vtkF3DOCCTReader::FILE_FORMAT::IGES);
}
