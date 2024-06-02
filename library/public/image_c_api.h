#ifndef F3D_C_API_H
#define F3D_C_API_H

#include "export.h" // Ensure F3D_EXPORT is defined

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
    float y;
    float z;
} f3d_point3_t;

typedef struct {
    float x;
    float y;
    float z;
} f3d_vector3_t;

typedef struct f3d_image f3d_image_t;

F3D_EXPORT f3d_image_t* f3d_image_new(void);
F3D_EXPORT void f3d_image_delete(f3d_image_t* img);
F3D_EXPORT void f3d_image_set_resolution(f3d_image_t* img, unsigned int width, unsigned int height);
F3D_EXPORT unsigned int f3d_image_get_width(f3d_image_t* img);
F3D_EXPORT unsigned int f3d_image_get_height(f3d_image_t* img);
F3D_EXPORT unsigned int f3d_image_get_channel_count(f3d_image_t* img);
F3D_EXPORT unsigned int f3d_image_get_channel_type(f3d_image_t* img);
F3D_EXPORT unsigned int f3d_image_get_channel_type_size(f3d_image_t* img);
F3D_EXPORT void f3d_image_set_content(f3d_image_t* img, void* buffer);
F3D_EXPORT void* f3d_image_get_content(f3d_image_t* img);
F3D_EXPORT int f3d_image_compare(f3d_image_t* img, f3d_image_t* reference, double threshold, f3d_image_t* diff, double* error);
F3D_EXPORT void f3d_image_save(f3d_image_t* img, const char* path, int format);
F3D_EXPORT unsigned char* f3d_image_save_buffer(f3d_image_t* img, int format, unsigned int* size);
F3D_EXPORT const char* f3d_image_to_terminal_text(f3d_image_t* img);
F3D_EXPORT void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value);
F3D_EXPORT const char* f3d_image_get_metadata(f3d_image_t* img, const char* key);
F3D_EXPORT char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count);
F3D_EXPORT void f3d_image_free_metadata_keys(char** keys, unsigned int count);
F3D_EXPORT f3d_image_t* f3d_image_create_from_file(const char* path);
F3D_EXPORT f3d_image_t* f3d_image_create_with_params(unsigned int width, unsigned int height, unsigned int channelCount, unsigned int type);
F3D_EXPORT unsigned int f3d_image_get_supported_formats_count();
F3D_EXPORT const char** f3d_image_get_supported_formats();
F3D_EXPORT double* f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, unsigned int* count);
F3D_EXPORT void f3d_image_free_normalized_pixel(double* pixel);

#ifdef __cplusplus
}
#endif

#endif // F3D_C_API_H
