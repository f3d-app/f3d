void applyCustomImporter(vtkImporter* importer, const std::string& fileName) const override
{
  vtkF3DQuakeMDLImporter* mdlImporter = vtkF3DQuakeMDLImporter::SafeDownCast(importer);

  std::string optName = "QuakeMDL.skin_index";
  std::string dsOptStr = this->ReaderOptions.at(optName);

  int skinIndex = F3DUtils::ParseToInt(dsOptStr, 0, optName);
  if (skinIndex < 0)
  {
    skinIndex = 0;
    vtkWarningWithObjectMacro(
      nullptr, "skinIndex must be positive. Defaulting to 0.");
  }
  mdlImporter->SetSkinIndex(skinIndex);
}
