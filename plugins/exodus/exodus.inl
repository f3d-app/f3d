void applyCustomReader(vtkAlgorithm* algo, const std::string&) const override
{
  vtkExodusIIReader* exReader = vtkExodusIIReader::SafeDownCast(algo);
  exReader->UpdateInformation();
  exReader->SetAllArrayStatus(vtkExodusIIReader::NODAL, 1);
  exReader->SetAllArrayStatus(vtkExodusIIReader::ELEM_BLOCK, 1);
}
