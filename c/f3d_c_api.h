
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

#ifdef __cplusplus
}
#endif

#endif 
