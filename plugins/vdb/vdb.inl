void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkOpenVDBReader* vdbReader = vtkOpenVDBReader::SafeDownCast(algo);
  vdbReader->UpdateInformation();

  // An arbitrary value that let us read sample files from OpenVDB in a reasonable time frame
  // exceptions and stuff
  double dsFactor = 0.1;
  std::string dsOptStr = this->PluginOptions->at("vdb.downsampling_factor");
  if (!dsOptStr.empty())
  {
    dsFactor = std::stod(dsOptStr);
  }

  vdbReader->SetDownsamplingFactor(dsFactor);

  // Merge volumes together
  vdbReader->MergeImageVolumesOn();
}
