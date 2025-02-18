void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkOpenVDBReader* vdbReader = vtkOpenVDBReader::SafeDownCast(algo);
  vdbReader->UpdateInformation();

  // An arbitrary value that let us read sample files from OpenVDB in a reasonable time frame
  vdbReader->SetDownsamplingFactor(0.1);

  // Merge volumes together
  vdbReader->MergeImageVolumesOn();
}
