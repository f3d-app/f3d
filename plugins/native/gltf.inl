void applyCustomReader(vtkAlgorithm* reader, const std::string&) const override
{
  vtkGLTFReader* gltfReader = vtkGLTFReader::SafeDownCast(reader);

  // Enable all animations in the GLTFReader
  gltfReader->SetFrameRate(30);
  gltfReader->ApplyDeformationsToGeometryOn();
  gltfReader->UpdateInformation(); // Read model metadata to get the number of animations
  for (vtkIdType i = 0; i < gltfReader->GetNumberOfAnimations(); i++)
  {
      gltfReader->EnableAnimation(i);
  }
  // It is needed to update the information directly in order to recover it later
  // Not entirely understood, TODO
  gltfReader->UpdateInformation();
}
