// clang-format off
void applyCustomReader(
  vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  std::string optName = "FCStd.linear_deflection";
  std::string str = this->ReaderOptions.at(optName);
  double linearDeflect = F3DUtils::ParseToDouble(str, 0.1, optName);

  optName = "FCStd.angular_deflection";
  str = this->ReaderOptions.at(optName);
  double angularDeflect = F3DUtils::ParseToDouble(str, 0.5, optName);

  optName = "FCStd.relative_deflection";
  str = this->ReaderOptions.at(optName);
  bool relativeDeflect = (F3DUtils::ParseToDouble(str, 0, optName) != 0);

  optName = "FCStd.read_wire";
  str = this->ReaderOptions.at(optName);
  bool readWire = (F3DUtils::ParseToDouble(str, 1, optName) != 0);

  vtkF3DFCStdReader* fcstdReader = vtkF3DFCStdReader::SafeDownCast(algo);
  fcstdReader->SetLinearDeflection(linearDeflect);
  fcstdReader->SetAngularDeflection(angularDeflect);
  fcstdReader->SetRelativeDeflection(relativeDeflect);
  fcstdReader->SetReadWire(readWire);
}
// clang-format on
