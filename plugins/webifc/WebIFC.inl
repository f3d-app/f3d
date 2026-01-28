void applyCustomReader(
  vtkAlgorithm* algo, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkF3DWebIFCReader* webIfcReader = vtkF3DWebIFCReader::SafeDownCast(algo);

  std::string optName = "WebIFC.circle_segments";
  std::string str = this->ReaderOptions.at(optName);
  int circleSegments = static_cast<int>(F3DUtils::ParseToDouble(str, 12, optName));
  webIfcReader->SetCircleSegments(circleSegments);

  optName = "WebIFC.read_openings";
  str = this->ReaderOptions.at(optName);
  bool readOpenings = (F3DUtils::ParseToDouble(str, 0, optName) != 0);
  webIfcReader->SetReadOpenings(readOpenings);

  optName = "WebIFC.read_spaces";
  str = this->ReaderOptions.at(optName);
  bool readSpaces = (F3DUtils::ParseToDouble(str, 0, optName) != 0);
  webIfcReader->SetReadSpaces(readSpaces);
}
