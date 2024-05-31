#ifndef F3D_C_API_H
#define F3D_C_API_H

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

f3d_image_t* f3d_image_new(void);
void f3d_image_delete(f3d_image_t* img);
void f3d_image_set_resolution(f3d_image_t* img, unsigned int width, unsigned int height);
unsigned int f3d_image_get_width(f3d_image_t* img);
unsigned int f3d_image_get_height(f3d_image_t* img);
unsigned int f3d_image_get_channel_count(f3d_image_t* img);
unsigned int f3d_image_get_channel_type(f3d_image_t* img);
unsigned int f3d_image_get_channel_type_size(f3d_image_t* img);
void f3d_image_set_content(f3d_image_t* img, void* buffer);
void* f3d_image_get_content(f3d_image_t* img);
int f3d_image_compare(f3d_image_t* img, f3d_image_t* reference, double threshold, f3d_image_t* diff, double* error);
void f3d_image_save(f3d_image_t* img, const char* path, int format);
unsigned char* f3d_image_save_buffer(f3d_image_t* img, int format, unsigned int* size);
const char* f3d_image_to_terminal_text(f3d_image_t* img);
void f3d_image_set_metadata(f3d_image_t* img, const char* key, const char* value);
const char* f3d_image_get_metadata(f3d_image_t* img, const char* key);
char** f3d_image_all_metadata(f3d_image_t* img, unsigned int* count);

#ifdef __cplusplus
}
#endif

#endif // F3D_C_API_H
