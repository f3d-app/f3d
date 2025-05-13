void applyCustomImporter(vtkImporter* importer, const std::string& fileName) const override
{
  vtkF3DQuakeMDLImporter* mdlImporter = vtkF3DQuakeMDLImporter::SafeDownCast(importer);

  std::string optName = "QuakeMDL.skin_index";
  std::string dsOptStr = this->ReaderOptions.at(optName);

  unsigned int skinIndex = F3DUtils::ParseToUInt(dsOptStr, 0, optName);
  mdlImporter->setSkinIndex(skinIndex);
}
