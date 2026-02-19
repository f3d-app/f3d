void applyCustomImporter(vtkImporter* importer, const std::string& vtkNotUsed(fileName), vtkResourceStream*) const override
{
  vtk3DSImporter::SafeDownCast(importer)->ComputeNormalsOn();
}
