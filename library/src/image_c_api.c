#include "image_c_api.h"
#include "image.h"

struct f3d_image {
    f3d::image img;
};

f3d_image_t* f3d_image_new(void) {
    return new f3d_image_t();
}

void f3d_image_delete(f3d_image_t* img) {
    delete img;
}

void f3d_image_set_resolution(f3d_image_t* img, unsigned int width, unsigned int height) {
    img->img.setResolution(width, height);
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
    return static_cast<unsigned int>(img->img.getChannelType());
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

int f3d_image_compare(f3d_image_t* img, f3d_image_t* reference, double threshold, f3d_image_t* diff, double* error) {
    return img->img.compare(reference->img, threshold, diff->img, *error);
}

void f3d_image_save(f3d_image_t* img, const char* path, int format) {
    f3d::image::SaveFormat save_format;
    switch (format) {
        case 0:
            save_format = f3d::image::SaveFormat::PNG;
            break;
        case 1:
            save_format = f3d::image::SaveFormat::JPG;
            break;
        case 2:
            save_format = f3d::image::SaveFormat::TIF;
            break;
        case 3:
            save_format = f3d::image::SaveFormat::BMP;
            break;
        default:
            save_format = f3d::image::SaveFormat::PNG; // Default to PNG
            break;
    }
    img->img.save(path, save_format);
}

unsigned char* f3d_image_save_buffer(f3d_image_t* img, int format, unsigned int* size) {
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
