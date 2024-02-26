void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkGLTFReader* gltfReader = vtkGLTFReader::SafeDownCast(algo);

  // Enable all animations in the GLTFReader
  // Specifying a non-zero framerate in the next call is not needed after VTK 9.2.20230603 : VTK_VERSION_CHECK(9, 2, 20230603)
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
