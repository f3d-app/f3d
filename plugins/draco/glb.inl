void applyCustomImporter([[maybe_unused]] vtkImporter* importer, const std::string&, vtkResourceStream*) const override
{
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 5, 20250204)
  vtkGLTFImporter* gltfImporter = vtkGLTFImporter::SafeDownCast(importer);
  gltfImporter->StreamIsBinaryOn();
#endif
}
