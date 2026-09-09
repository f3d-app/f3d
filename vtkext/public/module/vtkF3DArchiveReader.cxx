#include "vtkF3DArchiveReader.h"

#include <vtkObjectFactory.h>
#include <vtkResourceStream.h>
#include <vtkSmartPointer.h>

#include <miniz.h>

//----------------------------------------------------------------------------
class vtkF3DArchiveReader::vtkInternals
{
public:
  ~vtkInternals()
  {
    this->Close();
  }

  bool Open(vtkResourceStream* stream)
  {
    this->Close();
    if (!stream || !stream->SupportSeek())
    {
      return false;
    }
    this->Stream = stream;
    stream->Seek(0, vtkResourceStream::SeekDirection::End);
    const mz_uint64 size = static_cast<mz_uint64>(stream->Tell());

    mz_zip_zero_struct(&this->Archive);
    this->Archive.m_pRead = &vtkInternals::ReadCallback;
    this->Archive.m_pIO_opaque = this;
    this->Opened = mz_zip_reader_init(&this->Archive, size, 0);
    if (!this->Opened)
    {
      this->Stream = nullptr;
    }
    return this->Opened;
  }

  void Close()
  {
    if (this->Opened)
    {
      mz_zip_reader_end(&this->Archive);
      this->Opened = false;
    }
    this->Stream = nullptr;
  }

  bool IsOpened() const
  {
    return this->Opened;
  }

  int Locate(const std::string& name)
  {
    if (!this->Opened)
    {
      return -1;
    }
    return mz_zip_reader_locate_file(&this->Archive, name.c_str(), nullptr, 0);
  }

  bool Extract(int index, std::vector<char>& out)
  {
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&this->Archive, index, &stat))
    {
      return false;
    }
    out.resize(static_cast<size_t>(stat.m_uncomp_size));
    return mz_zip_reader_extract_to_mem(&this->Archive, index, out.data(), out.size(), 0);
  }

private:
  static size_t ReadCallback(void* opaque, mz_uint64 fileOfs, void* buf, size_t n)
  {
    vtkInternals* self = static_cast<vtkInternals*>(opaque);
    self->Stream->Seek(static_cast<vtkTypeInt64>(fileOfs), vtkResourceStream::SeekDirection::Begin);
    return self->Stream->Read(buf, n);
  }

  mz_zip_archive Archive = {};
  vtkSmartPointer<vtkResourceStream> Stream;
  bool Opened = false;
};

vtkStandardNewMacro(vtkF3DArchiveReader);

//----------------------------------------------------------------------------
vtkF3DArchiveReader::vtkF3DArchiveReader()
  : Internals(std::make_unique<vtkF3DArchiveReader::vtkInternals>())
{
}

//----------------------------------------------------------------------------
vtkF3DArchiveReader::~vtkF3DArchiveReader() = default;

//----------------------------------------------------------------------------
bool vtkF3DArchiveReader::Open(vtkResourceStream* stream)
{
  return this->Internals->Open(stream);
}

//----------------------------------------------------------------------------
void vtkF3DArchiveReader::Close()
{
  this->Internals->Close();
}

//----------------------------------------------------------------------------
bool vtkF3DArchiveReader::Has(const std::string& name)
{
  return this->Internals->Locate(name) >= 0;
}

//----------------------------------------------------------------------------
bool vtkF3DArchiveReader::Extract(const std::string& name, std::vector<char>& out)
{
  const int index = this->Internals->Locate(name);
  if (index < 0)
  {
    return false;
  }
  return this->Internals->Extract(index, out);
}

//----------------------------------------------------------------------------
void vtkF3DArchiveReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Opened: " << this->Internals->IsOpened() << "\n";
}
