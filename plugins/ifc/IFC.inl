void applyCustomReader(
  vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkF3DIFCReader* ifcReader = vtkF3DIFCReader::SafeDownCast(algo);

  std::string optName = "IFC.circle_segments";
  std::string str = this->ReaderOptions.at(optName);
  int circleSegments = static_cast<int>(F3DUtils::ParseToDouble(str, 12, optName));
  ifcReader->SetCircleSegments(circleSegments);

  optName = "IFC.read_openings";
  str = this->ReaderOptions.at(optName);
  bool readOpenings = (F3DUtils::ParseToDouble(str, 0, optName) != 0);
  ifcReader->SetReadOpenings(readOpenings);

  optName = "IFC.read_spaces";
  str = this->ReaderOptions.at(optName);
  bool readSpaces = (F3DUtils::ParseToDouble(str, 0, optName) != 0);
  ifcReader->SetReadSpaces(readSpaces);
}
