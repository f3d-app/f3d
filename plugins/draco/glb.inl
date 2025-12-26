void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtkGLTFImporter* gltfImporter = vtkGLTFImporter::SafeDownCast(importer);
  gltfImporter->StreamIsBinaryOn();
}
