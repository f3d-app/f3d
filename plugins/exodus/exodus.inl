void applyCustomReader(vtkAlgorithm* reader, const std::string&) const override
{
  vtkExodusIIReader* exReader = vtkExodusIIReader::SafeDownCast(reader);
  exReader->UpdateInformation();
  exReader->SetAllArrayStatus(vtkExodusIIReader::NODAL, 1);
  exReader->SetAllArrayStatus(vtkExodusIIReader::ELEM_BLOCK, 1);
}
