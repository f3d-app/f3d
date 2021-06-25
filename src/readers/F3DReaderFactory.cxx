#include "F3DReaderFactory.h"

//----------------------------------------------------------------------------
F3DReaderFactory* F3DReaderFactory::GetInstance()
{
  static F3DReaderFactory instance;
  return &instance;
}

//----------------------------------------------------------------------------
const std::list<F3DReader*>& F3DReaderFactory::GetReaders()
{
  return F3DReaderFactory::GetInstance()->Readers;
}

//----------------------------------------------------------------------------
F3DReader* F3DReaderFactory::GetReader(const std::string& fileName)
{
  for (auto reader : F3DReaderFactory::GetInstance()->Readers)
  {
    if (reader->CanRead(fileName))
    {
      return reader;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
F3DReader* F3DReaderFactory::GetReaderByName(const std::string& name)
{
  for (auto reader : F3DReaderFactory::GetInstance()->Readers)
  {
    if (reader->GetName() == name)
    {
      return reader;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
void F3DReaderFactory::Register(F3DReader* reader)
{
  F3DReaderFactory::GetInstance()->Readers.push_back(reader);
}

//----------------------------------------------------------------------------
void F3DReaderFactory::Unregister(F3DReader* reader)
{
  F3DReaderFactory::GetInstance()->Readers.remove(reader);
}
