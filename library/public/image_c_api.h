#ifndef F3D_C_API_H
#define F3D_C_API_H

#include "export.h" // Ensure F3D_EXPORT is defined

#ifdef __cplusplus
extern "C" {
#endif

// Structure representing a 3D point
typedef struct {
    float x;
    float y;
    float z;
} f3d_point3_t;

// Structure representing a 3D vector
typedef struct {
    float x;
    float y;
    float z;
} f3d_vector3_t;

// Forward declaration of the f3d_image structure
typedef struct f3d_image f3d_image_t;

// Function to create a new image object
F3D_EXPORT f3d_image_t* f3d_image_new(void);

// Function to delete an image object
F3D_EXPORT void f3d_image_delete(f3d_image_t* img);

// Function to set the resolution of an image
F3D_EXPORT void f3d_image_set_resolution(f3d_image_t* img, unsigned int width, unsigned int height);

// Function to get the width of an image
F3D_EXPORT unsigned int f3d_image_get_width(f3d_image_t* img);

// Function to get the height of an image
F3D_EXPORT unsigned int f3d_image_get_height(f3d_image_t* img);

// Function to get the number of channels in an image
F3D_EXPORT unsigned int f3d_image_get_channel_count(f3d_image_t* img);

// Function to get the type of channels in an image
F3D_EXPORT unsigned int f3d_image_get_channel_type(f3d_image_t* img);

// Function to get the size of the channel type in an image
F3D_EXPORT unsigned int f3d_image_get_channel_type_size(f3d_image_t* img);

// Function to set the content of an image from a buffer
F3D_EXPORT void f3d_image_set_content(f3d_image_t* img, void* buffer);

// Function to get the content of an image as a buffer
F3D_EXPORT void* f3d_image_get_content(f3d_image_t* img);

// Function to compare two images and return the difference
F3D_EXPORT int f3d_image_compare(f3d_image_t* img, f3d_image_t* reference, double threshold, f3d_image_t* diff, double* error);

// Function to save an image to a file
F3D_EXPORT void f3d_image_save(f3d_image_t* img, const char* path, int format);

// Function to save an image to a buffer
F3D_EXPORT unsigned char* f3d_image_save_buffer(f3d_image_t* img, int format, unsigned int* size);

// Function to convert an image to a string representation for terminal output
F3D_EXPORT const char* f3d_image_to_terminal_text(f3d_image_t* img);

// Function to set metadata for an image
F3D_EXPORT void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value);

// Function to get metadata from an image
F3D_EXPORT const char* f3d_image_get_metadata(f3d_image_t* img, const char* key);

// Function to get all metadata keys from an image
F3D_EXPORT char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count);

// Function to free metadata keys obtained from an image
F3D_EXPORT void f3d_image_free_metadata_keys(char** keys, unsigned int count);

// Function to create an image from a file
F3D_EXPORT f3d_image_t* f3d_image_create_from_file(const char* path);

// Function to create an image with specified parameters
F3D_EXPORT f3d_image_t* f3d_image_create_with_params(unsigned int width, unsigned int height, unsigned int channelCount, unsigned int type);

// Function to get the count of supported image formats
F3D_EXPORT unsigned int f3d_image_get_supported_formats_count();

// Function to get the list of supported image formats
F3D_EXPORT const char** f3d_image_get_supported_formats();

// Function to get a normalized pixel value from an image
F3D_EXPORT double* f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, unsigned int* count);

// Function to free a normalized pixel value obtained from an image
F3D_EXPORT void f3d_image_free_normalized_pixel(double* pixel);

#ifdef __cplusplus
}
#endif

#endif // F3D_C_API_H
