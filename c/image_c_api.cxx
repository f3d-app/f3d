#include "image_c_api.h"
#include "image.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>

// Helper class to wrap FILE* as an ostream
class FileStreamBuf : public std::streambuf
{
public:
  explicit FileStreamBuf(FILE* file)
    : file_(file)
  {
  }

protected:
  int_type overflow(int_type c) override
  {
    if (c != EOF)
    {
      if (std::fputc(c, file_) == EOF)
      {
        return EOF;
      }
    }
    return c;
  }

  std::streamsize xsputn(const char* s, std::streamsize n) override
  {
    return std::fwrite(s, 1, n, file_);
  }

private:
  FILE* file_;
};

//----------------------------------------------------------------------------
f3d_image_t* f3d_image_new_empty()
{
  f3d::image* img = new f3d::image();
  return reinterpret_cast<f3d_image_t*>(img);
}

//----------------------------------------------------------------------------
f3d_image_t* f3d_image_new_params(unsigned int width, unsigned int height,
  unsigned int channelCount, f3d_image_channel_type_t channelType)
{
  f3d::image* img =
    new f3d::image(width, height, channelCount, static_cast<f3d::image::ChannelType>(channelType));
  return reinterpret_cast<f3d_image_t*>(img);
}

//----------------------------------------------------------------------------
f3d_image_t* f3d_image_new_path(const char* path)
{
  f3d::image* img = new f3d::image(path);
  return reinterpret_cast<f3d_image_t*>(img);
}

//----------------------------------------------------------------------------
void f3d_image_delete(f3d_image_t* img)
{
  f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  delete cpp_img;
}

//----------------------------------------------------------------------------
void f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, double* pixel)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  std::vector<double> d = cpp_img->getNormalizedPixel({ x, y });
  std::copy(d.begin(), d.end(), pixel);
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_width(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return cpp_img->getWidth();
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_height(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return cpp_img->getHeight();
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_channel_count(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return cpp_img->getChannelCount();
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_channel_type(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return static_cast<f3d_image_channel_type_t>(cpp_img->getChannelType());
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_channel_type_size(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return cpp_img->getChannelTypeSize();
}

//----------------------------------------------------------------------------
void f3d_image_set_content(f3d_image_t* img, void* buffer)
{
  f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  cpp_img->setContent(buffer);
}

//----------------------------------------------------------------------------
void* f3d_image_get_content(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  return cpp_img->getContent();
}

//----------------------------------------------------------------------------
double f3d_image_compare(f3d_image_t* img, f3d_image_t* reference)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  const f3d::image* cpp_ref = reinterpret_cast<f3d::image*>(reference);
  return cpp_img->compare(*cpp_ref);
}

//----------------------------------------------------------------------------
int f3d_image_equals(f3d_image_t* img, f3d_image_t* reference)
{
  if (!img || !reference)
  {
    return 0;
  }

  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  const f3d::image* cpp_ref = reinterpret_cast<f3d::image*>(reference);
  return *cpp_img == *cpp_ref ? 1 : 0;
}

//----------------------------------------------------------------------------
int f3d_image_not_equals(f3d_image_t* img, f3d_image_t* reference)
{
  if (!img || !reference)
  {
    return 1;
  }

  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  const f3d::image* cpp_ref = reinterpret_cast<f3d::image*>(reference);
  return *cpp_img != *cpp_ref ? 1 : 0;
}

//----------------------------------------------------------------------------
void f3d_image_save(f3d_image_t* img, const char* path, f3d_image_save_format_t format)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  cpp_img->save(path, static_cast<f3d::image::SaveFormat>(format));
}

//----------------------------------------------------------------------------
unsigned char* f3d_image_save_buffer(
  f3d_image_t* img, f3d_image_save_format_t format, unsigned int* size)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  std::vector<unsigned char> buffer =
    cpp_img->saveBuffer(static_cast<f3d::image::SaveFormat>(format));
  unsigned char* c_buffer = new unsigned char[buffer.size()];
  std::copy(buffer.begin(), buffer.end(), c_buffer);
  *size = buffer.size();
  return c_buffer;
}

//----------------------------------------------------------------------------
void f3d_image_free_buffer(unsigned char* buffer)
{
  delete[] buffer;
}

//----------------------------------------------------------------------------
void f3d_image_to_terminal_text(f3d_image_t* img, void* stream)
{
  if (!img || !stream)
  {
    return;
  }

  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  FILE* file = static_cast<FILE*>(stream);

  FileStreamBuf buffer(file);
  std::ostream os(&buffer);

  cpp_img->toTerminalText(os);
  os.flush();
}

//----------------------------------------------------------------------------
const char* f3d_image_to_terminal_text_string(f3d_image_t* img)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  static std::string result;
  result = cpp_img->toTerminalText();
  return result.c_str();
}

//----------------------------------------------------------------------------
void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value)
{
  f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  cpp_img->setMetadata(key, value);
}

//----------------------------------------------------------------------------
const char* f3d_image_get_metadata(f3d_image_t* img, const char* key)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  static std::string result;
  result = cpp_img->getMetadata(key);
  return result.c_str();
}

//----------------------------------------------------------------------------
char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count)
{
  const f3d::image* cpp_img = reinterpret_cast<f3d::image*>(img);
  std::vector<std::string> metadata_keys = cpp_img->allMetadata();
  *count = metadata_keys.size();
  char** keys = new char*[metadata_keys.size()];
  for (size_t i = 0; i < metadata_keys.size(); ++i)
  {
    keys[i] = new char[metadata_keys[i].size() + 1];
    std::strcpy(keys[i], metadata_keys[i].c_str());
  }
  return keys;
}

//----------------------------------------------------------------------------
void f3d_image_free_metadata_keys(char** keys, unsigned int count)
{
  for (unsigned int i = 0; i < count; ++i)
  {
    delete[] keys[i];
  }
  delete[] keys;
}

//----------------------------------------------------------------------------
unsigned int f3d_image_get_supported_formats_count()
{
  std::vector<std::string> formats = f3d::image::getSupportedFormats();
  return formats.size();
}

//----------------------------------------------------------------------------
const char** f3d_image_get_supported_formats()
{
  static std::vector<std::string> formats = f3d::image::getSupportedFormats();
  static std::vector<const char*> c_formats;
  c_formats.clear();
  c_formats.resize(formats.size());
  std::transform(formats.begin(), formats.end(), c_formats.begin(),
    [](const std::string& s) { return s.c_str(); });
  return c_formats.data();
}
