void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkOpenVDBReader* vdbReader = vtkOpenVDBReader::SafeDownCast(algo);
  vdbReader->UpdateInformation();

  // An arbitrary value that let us read sample files from OpenVDB in a reasonable time frame
  // exceptions and stuff
  double dsFactor = 0.1;

  // No check needed, we know the option exists
  std::string dsOptStr = this->ReaderOptions.at("VDB.downsampling_factor");
  if (!dsOptStr.empty())
  {
    try
    {
      dsFactor = std::stod(dsOptStr);
    }
    catch (const std::invalid_argument&)
    {
      vtkWarningWithObjectMacro(vdbReader, "Could not parse vdb.downsampling_factor: " << dsOptStr << ". Ignoring.");
    }
    catch (const std::out_of_range&)
    {
      vtkWarningWithObjectMacro(vdbReader, "vdb.downsampling_factor out of range: " << dsOptStr << ". Ignoring.");
    }
  }

  vdbReader->SetDownsamplingFactor(dsFactor);

  // Merge volumes together
  vdbReader->MergeImageVolumesOn();
}
