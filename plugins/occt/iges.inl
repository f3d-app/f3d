void applyCustomReader(vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName)) const override
{
  vtkOCCTReader* occtReader = vtkOCCTReader::SafeDownCast(algo);

  occtReader->RelativeDeflectionOn();
  occtReader->SetLinearDeflection(0.1);
  occtReader->SetAngularDeflection(0.5);
  occtReader->ReadWireOn();
  occtReader->SetFileFormat(vtkOCCTReader::IGES);
}
