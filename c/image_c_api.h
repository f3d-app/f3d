#ifndef F3D_C_API_H
#define F3D_C_API_H

#include "export.h" // Ensure F3D_EXPORT is defined

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum { PNG, JPG, TIF, BMP } f3d_image_save_format_t;

typedef enum { BYTE, SHORT, FLOAT } f3d_image_channel_type_t;

/**
* @struct f3d_image_t
* @brief Forward declaration of the f3d_image structure
*/
typedef struct f3d_image f3d_image_t;

/**
* @brief Create a new empty image object
* @return Pointer to the newly created image object
*/
F3D_EXPORT f3d_image_t* f3d_image_new_empty();

/**
* @brief Create a new image object with the given parameters
* @return Pointer to the newly created image object
*/
F3D_EXPORT f3d_image_t* f3d_image_new_params(unsigned int width, unsigned int height,
unsigned int channelCount, f3d_image_channel_type_t channelType);

/**
* @brief Create a new image object from a file path
* @return Pointer to the newly created image object
*/
F3D_EXPORT f3d_image_t* f3d_image_new_path(const char* path);

/**
* @brief Delete an image object
* @param img Pointer to the image object to be deleted
*/
F3D_EXPORT void f3d_image_delete(f3d_image_t* img);

/**
* @brief Get the normalized pixel
* @param img Pointer to the image object to be deleted
* @param x horizontal pixel coordinate
* @param y vertical pixel coordinate
* @param pixel Pointer to a preallocated buffer of channel count size
*/
F3D_EXPORT void f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, double* pixel);

/**
* @brief Get the width of an image
* @param img Pointer to the image object
* @return Width of the image
*/
F3D_EXPORT unsigned int f3d_image_get_width(f3d_image_t* img);

/**
* @brief Get the height of an image
* @param img Pointer to the image object
* @return Height of the image
*/
F3D_EXPORT unsigned int f3d_image_get_height(f3d_image_t* img);

/**
* @brief Get the number of channels in an image
* @param img Pointer to the image object
* @return Number of channels in the image
*/
F3D_EXPORT unsigned int f3d_image_get_channel_count(f3d_image_t* img);

/**
* @brief Get the type of channels in an image
* @param img Pointer to the image object
* @return Type of channels in the image
*/
F3D_EXPORT unsigned int f3d_image_get_channel_type(f3d_image_t* img);

/**
* @brief Get the size of the channel type in an image
* @param img Pointer to the image object
* @return Size of the channel type in the image
*/
F3D_EXPORT unsigned int f3d_image_get_channel_type_size(f3d_image_t* img);

/**
* @brief Set the content of an image from a buffer
* @param img Pointer to the image object
* @param buffer Pointer to the buffer containing the image content
*/
F3D_EXPORT void f3d_image_set_content(f3d_image_t* img, void* buffer);

/**
* @brief Get the content of an image as a buffer
* @param img Pointer to the image object
* @return Pointer to the buffer containing the image content
*/
F3D_EXPORT void* f3d_image_get_content(f3d_image_t* img);

/**
* @brief Compare two images
* @param img Pointer to the image object
* @param reference Pointer to the reference image object
* @return SSIM difference between the two images
*/
F3D_EXPORT double f3d_image_compare(f3d_image_t* img, f3d_image_t* reference);

/**
* @brief Save an image to a file
* @param img Pointer to the image object
* @param path Path to the file where the image will be saved
* @param format Format in which the image will be saved
*/
F3D_EXPORT void f3d_image_save(
f3d_image_t* img, const char* path, f3d_image_save_format_t format);

/**
* @brief Save an image to a buffer
* @param img Pointer to the image object
* @param format Format in which the image will be saved
* @param size Pointer to store the size of the saved buffer
* @return Pointer to the buffer containing the saved image
*/
F3D_EXPORT unsigned char* f3d_image_save_buffer(
f3d_image_t* img, f3d_image_save_format_t format, unsigned int* size);

/**
* @brief Convert an image to a string representation for terminal output
* @param img Pointer to the image object
* @return Pointer to the string representation of the image
*/
F3D_EXPORT const char* f3d_image_to_terminal_text(f3d_image_t* img);

/**
* @brief Set metadata for an image
* @param img Pointer to the image object
* @param key Metadata key
* @param value Metadata value
*/
F3D_EXPORT void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value);

/**
* @brief Get metadata from an image
* @param img Pointer to the image object
* @param key Metadata key
* @return Metadata value
*/
F3D_EXPORT const char* f3d_image_get_metadata(f3d_image_t* img, const char* key);

/**
* @brief Get all metadata keys from an image
* @param img Pointer to the image object
* @param count Pointer to store the count of metadata keys
* @return Pointer to the array of metadata keys
*/
F3D_EXPORT char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count);

/**
* @brief Free metadata keys obtained from an image
* @param keys Pointer to the array of metadata keys
* @param count Count of metadata keys
*/
F3D_EXPORT void f3d_image_free_metadata_keys(char** keys, unsigned int count);

/**
* @brief Create an image from a file
* @param path Path to the image file
* @return Pointer to the created image object
*/
F3D_EXPORT f3d_image_t* f3d_image_create_from_file(const char* path);

/**
* @brief Create an image with specified parameters
* @param width Width of the image
* @param height Height of the image
* @param channelCount Number of channels in the image
* @param type Type of channels in the image
* @return Pointer to the created image object
*/
F3D_EXPORT f3d_image_t* f3d_image_create_with_params(
unsigned int width, unsigned int height, unsigned int channelCount, unsigned int type);

/**
* @brief Get the count of supported image formats
* @return Count of supported image formats
*/
F3D_EXPORT unsigned int f3d_image_get_supported_formats_count();

/**
* @brief Get the list of supported image formats
* @return Pointer to the array of supported image formats
*/
F3D_EXPORT const char** f3d_image_get_supported_formats();

/**
* @brief Free a normalized pixel value obtained from an image
* @param pixel Pointer to the array of normalized pixel values
*/
F3D_EXPORT void f3d_image_free_normalized_pixel(double* pixel);

#ifdef __cplusplus
}
#endif

#endif // F3D_C_API_H
