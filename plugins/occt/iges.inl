void applyCustomReader(vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName)) const override
{
  std::string optName = "IGES.linear_deflection";
  std::string str = this->ReaderOptions.at(optName);
  double linearDeflect = F3DUtils::ParseToDouble(str, 0.1, optName);

  optName = "IGES.angular_deflection";
  str = this->ReaderOptions.at(optName);
  double angularDeflect = F3DUtils::ParseToDouble(str, 0.5, optName);

  vtkF3DOCCTReader* occtReader = vtkF3DOCCTReader::SafeDownCast(algo);
  occtReader->RelativeDeflectionOn();
  occtReader->SetLinearDeflection(linearDeflect);
  occtReader->SetAngularDeflection(angularDeflect);
  occtReader->ReadWireOn();
  occtReader->SetFileFormat(vtkF3DOCCTReader::FILE_FORMAT::IGES);
}
