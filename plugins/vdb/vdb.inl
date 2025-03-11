void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkOpenVDBReader* vdbReader = vtkOpenVDBReader::SafeDownCast(algo);
  vdbReader->UpdateInformation();

  // No check needed, we know the option exists
  std::string optName = "VDB.downsampling_factor";
  std::string dsOptStr = this->ReaderOptions.at(optName);

  // 0.1 is an arbitrary default that let us read sample files from OpenVDB in a reasonable time frame
  double dsFactor = F3DUtils::ParseToDouble(dsOptStr, 0.1, optName);
  vdbReader->SetDownsamplingFactor(dsFactor);

  // Merge volumes together
  vdbReader->MergeImageVolumesOn();
}
