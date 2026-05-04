// Custom code for Open3SDCM reader
// This file provides the canRead() implementations for the reader

#include "ParseDcm.h"

#include <vtkFileResourceStream.h>

#include <fstream>
#include <string>

namespace {

// Helper function to extract file extension
std::string GetFileExtension(const std::string& filename)
{
  size_t pos = filename.find_last_of(".");
  if (pos != std::string::npos)
  {
    return filename.substr(pos + 1);
  }
  return "";
}

// Helper function to check if file starts with specific string
bool FileStartsWith(const std::string& filename, const char* str, size_t len)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file)
  {
    return false;
  }

  char buffer[32];
  if (!file.read(buffer, len) || static_cast<size_t>(file.gcount()) < len)
  {
    return false;
  }

  return memcmp(buffer, str, len) == 0;
}

// Helper function to check if stream starts with specific string
bool StreamStartsWith(vtkResourceStream* stream, const char* str, size_t len)
{
  if (!stream || !stream->Open())
  {
    return false;
  }

  char buffer[32];
  if (stream->Read(buffer, len) != static_cast<std::streamsize>(len))
  {
    stream->Close();
    return false;
  }

  stream->Close();
  return memcmp(buffer, str, len) == 0;
}

} // namespace

// Implementation of canRead for file path
bool canRead(const std::string& fileName) const override
{
  // First check extension
  std::string ext = GetFileExtension(fileName);
  if (ext != "dcm")
  {
    return false;
  }

  // Check for DICOM header - explicitly reject DICOM files
  // DICOM files start with "DICM"
  const char dicomHeader[] = { 'D', 'I', 'C', 'M' };
  if (FileStartsWith(fileName, dicomHeader, 4))
  {
    return false; // It's DICOM, not 3Shape
  }

  // Check for 3Shape DCM - starts with "<HPS version=" or "<HPS"
  // This is the most reliable indicator as 3Shape DCM files are XML
  const char hpsHeaderFull[] = "<HPS version=";
  const char hpsHeaderShort[] = "<HPS";
  if (FileStartsWith(fileName, hpsHeaderFull, 12) || FileStartsWith(fileName, hpsHeaderShort, 5))
  {
    return true; // It's 3Shape DCM
  }

  // Some 3Shape DCM files might be ZIP archives containing HPS XML
  // Check for ZIP header as fallback
  const char zipHeader[] = { 'P', 'K', '\x03', '\x04' };
  if (FileStartsWith(fileName, zipHeader, 4))
  {
    return true; // It's a ZIP, could be 3Shape DCM
  }

  // Unknown format - let other readers try
  return false;
}

// Implementation of canRead for stream
bool canRead(vtkResourceStream* stream) const override
{
  if (!stream)
  {
    return false;
  }

  // Check for 3Shape DCM - starts with "<HPS version=" or "<HPS"
  const char hpsHeaderFull[] = "<HPS version=";
  const char hpsHeaderShort[] = "<HPS";
  if (StreamStartsWith(stream, hpsHeaderFull, 12) || StreamStartsWith(stream, hpsHeaderShort, 5))
  {
    return true;
  }

  // Check for ZIP header as fallback (older format)
  const char zipHeader[] = { 'P', 'K', '\x03', '\x04' };
  if (StreamStartsWith(stream, zipHeader, 4))
  {
    return true;
  }

  return false;
}

// Implementation of applyCustomReader (optional customization)
// This is called after the reader is created to allow custom configuration
void applyCustomReader(vtkAlgorithm* algo, const std::string& fileName, vtkResourceStream*) const override
{
  // Cast to our reader type
  auto* reader = vtkF3DOpen3SDCMReader::SafeDownCast(algo);
  if (!reader)
  {
    return;
  }

  // Set the filename if provided
  if (!fileName.empty())
  {
    reader->SetFileName(fileName.c_str());
  }
}
