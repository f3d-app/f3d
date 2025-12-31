#ifndef F3D_IMAGE_C_API_H
#define F3D_IMAGE_C_API_H

#include "export.h" // Ensure F3D_EXPORT is defined

#ifdef __cplusplus
extern "C"
{
#endif

  typedef enum
  {
    PNG,
    JPG,
    TIF,
    BMP
  } f3d_image_save_format_t;

  typedef enum
  {
    BYTE,
    SHORT,
    FLOAT
  } f3d_image_channel_type_t;

  /**
   * @struct f3d_image_t
   * @brief Forward declaration of the f3d_image structure
   */
  typedef struct f3d_image f3d_image_t;

  /**
   * @brief Create a new empty image object
   *
   * The returned image must be deleted with f3d_image_delete().
   *
   * @return Pointer to the newly created image object
   */
  F3D_EXPORT f3d_image_t* f3d_image_new_empty();

  /**
   * @brief Create a new image object with the given parameters
   *
   * The returned image must be deleted with f3d_image_delete().
   *
   * @return Pointer to the newly created image object
   */
  F3D_EXPORT f3d_image_t* f3d_image_new_params(unsigned int width, unsigned int height,
    unsigned int channelCount, f3d_image_channel_type_t channelType);

  /**
   * @brief Create a new image object from a file path
   *
   * The returned image must be deleted with f3d_image_delete().
   *
   * @return Pointer to the newly created image object
   */
  F3D_EXPORT f3d_image_t* f3d_image_new_path(const char* path);

  /**
   * @brief Delete an image object
   * @param img Pointer to the image object to be deleted
   */
  F3D_EXPORT void f3d_image_delete(f3d_image_t* img);

  /**
   * @brief Test if two images are equal
   *
   * @param img Pointer to the first image object
   * @param reference Pointer to the second image object
   * @return Non-zero if images are equal, zero otherwise
   */
  F3D_EXPORT int f3d_image_equals(f3d_image_t* img, f3d_image_t* reference);

  /**
   * @brief Test if two images are not equal
   *
   * @param img Pointer to the first image object
   * @param reference Pointer to the second image object
   * @return Non-zero if images are not equal, zero otherwise
   */
  F3D_EXPORT int f3d_image_not_equals(f3d_image_t* img, f3d_image_t* reference);

  /**
   * @brief Get the normalized pixel
   * @param img Pointer to the image object to be deleted
   * @param x horizontal pixel coordinate
   * @param y vertical pixel coordinate
   * @param pixel Pointer to a preallocated buffer of channel count size
   */
  F3D_EXPORT void f3d_image_get_normalized_pixel(f3d_image_t* img, int x, int y, double* pixel);

  /**
   * @brief Get the count of supported image formats
   * @return Count of supported image formats
   */
  F3D_EXPORT unsigned int f3d_image_get_supported_formats_count();

  /**
   * @brief Get the list of supported image formats
   *
   * The returned array points to internal static storage and must NOT be freed.
   * The pointer is valid until the next call to this function.
   *
   * @return Pointer to the array of supported image formats
   */
  F3D_EXPORT const char** f3d_image_get_supported_formats();

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
   *
   * The returned pointer is owned by the image and must NOT be freed.
   * It is valid as long as the image exists and its content is not modified.
   *
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
   *
   * The returned buffer is heap-allocated and must be freed with f3d_image_free_buffer().
   *
   * @param img Pointer to the image object
   * @param format Format in which the image will be saved
   * @param size Pointer to store the size of the saved buffer
   * @return Pointer to the buffer containing the saved image
   */
  F3D_EXPORT unsigned char* f3d_image_save_buffer(
    f3d_image_t* img, f3d_image_save_format_t format, unsigned int* size);

  /**
   * @brief Free a buffer returned by f3d_image_save_buffer
   * @param buffer Pointer to the buffer to free
   */
  F3D_EXPORT void f3d_image_free_buffer(unsigned char* buffer);

  /**
   * @brief Convert an image to colored text using ANSI escape sequences for terminal output
   *
   * Writes colored text to the provided file stream.
   * This is the C equivalent of toTerminalText(std::ostream&).
   *
   * @param img Pointer to the image object
   * @param stream File stream to write to (e.g., stdout, stderr, or file handle)
   */
  F3D_EXPORT void f3d_image_to_terminal_text(f3d_image_t* img, void* stream);

  /**
   * @brief Convert an image to a string representation for terminal output
   *
   * The returned string points to internal static storage and must NOT be freed.
   * The pointer is valid until the next call to this function.
   * This is the C equivalent of toTerminalText() that returns a std::string.
   *
   * @param img Pointer to the image object
   * @return Pointer to the string representation of the image
   */
  F3D_EXPORT const char* f3d_image_to_terminal_text_string(f3d_image_t* img);

  /**
   * @brief Set metadata for an image
   * @param img Pointer to the image object
   * @param key Metadata key
   * @param value Metadata value
   */
  F3D_EXPORT void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value);

  /**
   * @brief Get metadata from an image
   *
   * The returned string points to internal static storage and must NOT be freed.
   * The pointer is valid until the next call to this function.
   *
   * @param img Pointer to the image object
   * @param key Metadata key
   * @return Metadata value
   */
  F3D_EXPORT const char* f3d_image_get_metadata(f3d_image_t* img, const char* key);

  /**
   * @brief Get all metadata keys from an image
   *
   * The returned keys must be freed with f3d_image_free_metadata_keys.
   *
   * @param img Pointer to the image object
   * @param count Pointer to store the count of metadata keys
   * @return Pointer to the array of metadata keys
   */
  F3D_EXPORT char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count);

  /**
   * @brief Free metadata keys obtained from an image
   *
   * Used to free the return of f3d_image_all_metadata.
   *
   * @param keys Pointer to the array of metadata keys
   * @param count Count of metadata keys
   */
  F3D_EXPORT void f3d_image_free_metadata_keys(char** keys, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif // F3D_IMAGE_C_API_H
