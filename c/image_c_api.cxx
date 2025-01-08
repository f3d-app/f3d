#include "image_c_api.h"
#include "image.h"
#include <cstring>

struct f3d_image {
    f3d::image img;
};

f3d_image_t* f3d_image_new_empty() {
    return new f3d_image_t{f3d::image()};
}

f3d_image_t* f3d_image_new_params(
    unsigned int width, unsigned int height, unsigned int channelCount, f3d_image_channel_type_t channelType)
{
    return new f3d_image_t{ f3d::image(width, height, channelCount, static_cast<f3d::image::ChannelType>(channelType)) };
}

f3d_image_t* f3d_image_new_path(const char* path)
{
    return new f3d_image_t{ f3d::image(path) };
}

void f3d_image_delete(f3d_image_t* img) {
    delete img;
}

void f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, double* pixel)
{
    std::vector<double> d = img->img.getNormalizedPixel({ x, y });
    std::copy(d.begin(), d.end(), pixel);
}

unsigned int f3d_image_get_width(f3d_image_t* img) {
    return img->img.getWidth();
}

unsigned int f3d_image_get_height(f3d_image_t* img) {
    return img->img.getHeight();
}

unsigned int f3d_image_get_channel_count(f3d_image_t* img) {
    return img->img.getChannelCount();
}

unsigned int f3d_image_get_channel_type(f3d_image_t* img) {
    return static_cast<f3d_image_channel_type_t>(img->img.getChannelType());
}

unsigned int f3d_image_get_channel_type_size(f3d_image_t* img) {
    return img->img.getChannelTypeSize();
}

void f3d_image_set_content(f3d_image_t* img, void* buffer) {
    img->img.setContent(buffer);
}

void* f3d_image_get_content(f3d_image_t* img) {
    return img->img.getContent();
}

double f3d_image_compare(f3d_image_t* img, f3d_image_t* reference) {
    return img->img.compare(reference->img);
}

void f3d_image_save(f3d_image_t* img, const char* path, f3d_image_save_format_t format)
{
    img->img.save(path, static_cast<f3d::image::SaveFormat>(format));
}

unsigned char* f3d_image_save_buffer(f3d_image_t* img, f3d_image_save_format_t format, unsigned int* size) {
    std::vector<unsigned char> buffer = img->img.saveBuffer(static_cast<f3d::image::SaveFormat>(format));
    unsigned char* c_buffer = new unsigned char[buffer.size()];
    std::copy(buffer.begin(), buffer.end(), c_buffer);
    *size = buffer.size();
    return c_buffer;
}

const char* f3d_image_to_terminal_text(f3d_image_t* img) {
    static std::string result;
    result = img->img.toTerminalText();
    return result.c_str();
}

void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value) {
    img->img.setMetadata(key, value);
}

const char* f3d_image_get_metadata(f3d_image_t* img, const char* key) {
    static std::string result;
    result = img->img.getMetadata(key);
    return result.c_str();
}

char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count) {
    std::vector<std::string> metadata_keys = img->img.allMetadata();
    *count = metadata_keys.size();
    char** keys = new char*[metadata_keys.size()];
    for (size_t i = 0; i < metadata_keys.size(); ++i) {
        keys[i] = new char[metadata_keys[i].size() + 1];
        strcpy(keys[i], metadata_keys[i].c_str());
    }
    return keys;
}

void f3d_image_free_metadata_keys(char** keys, unsigned int count) {
    for (unsigned int i = 0; i < count; ++i) {
        delete[] keys[i];
    }
    delete[] keys;
}

// Additional functions to match all functionalities from image.cxx
f3d_image_t* f3d_image_create_from_file(const char* path) {
    return new f3d_image_t{ f3d::image(path) };
}

f3d_image_t* f3d_image_create_with_params(unsigned int width, unsigned int height, unsigned int channelCount, unsigned int type) {
    f3d::image::ChannelType channel_type;
    switch (type) {
        case 0:
            channel_type = f3d::image::ChannelType::BYTE;
            break;
        case 1:
            channel_type = f3d::image::ChannelType::SHORT;
            break;
        case 2:
            channel_type = f3d::image::ChannelType::FLOAT;
            break;
        default:
            channel_type = f3d::image::ChannelType::BYTE; // Default to BYTE
            break;
    }
    return new f3d_image_t{ f3d::image(width, height, channelCount, channel_type) };
}

unsigned int f3d_image_get_supported_formats_count() {
    std::vector<std::string> formats = f3d::image::getSupportedFormats();
    return formats.size();
}

const char** f3d_image_get_supported_formats() {
    static std::vector<std::string> formats = f3d::image::getSupportedFormats();
    static std::vector<const char*> c_formats;
    c_formats.clear();
    for (const auto& format : formats) {
        c_formats.push_back(format.c_str());
    }
    return c_formats.data();
}

void f3d_image_free_normalized_pixel(double* pixel) {
    delete[] pixel;
}
