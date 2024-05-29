// f3d_c_api.c
#include "f3d_c_api.h"
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
