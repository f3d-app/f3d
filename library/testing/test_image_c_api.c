#include <f3d/image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    
    f3d_image_t* img = f3d_image_new();
   
    f3d_image_set_resolution(img, 800, 600);
    printf("Image resolution set to 800x600.\n");

    unsigned int width = f3d_image_get_width(img);
    unsigned int height = f3d_image_get_height(img);
    printf("Image width: %u, height: %u\n", width, height);

    unsigned int channels = f3d_image_get_channel_count(img);
    printf("Image channel count: %u\n", channels);

    size_t buffer_size = width * height * channels;
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    if (!buffer) {
        printf("Failed to allocate buffer.\n");
        f3d_image_delete(img);
        return 1;
    }
    memset(buffer, 255, buffer_size);  // Set all pixels to white
    f3d_image_set_content(img, buffer);
    printf("Image content set to white.\n");

    f3d_image_save(img, "output.png", 0);
    printf("Image saved as output.png\n");

    // Save image to buffer
    unsigned int size;
    unsigned char* save_buffer = f3d_image_save_buffer(img, 0, &size);
    if (save_buffer) {
        printf("Image saved to buffer, size: %u bytes.\n", size);
        free(save_buffer);
    } else {
        printf("Failed to save image to buffer.\n");
    }

    // Set metadata
    f3d_image_set_metadata(img, "Author", "TestUser");
    printf("Metadata 'Author' set to 'TestUser'.\n");

    // Get metadata
    const char* author = f3d_image_get_metadata(img, "Author");
    if (author) {
        printf("Metadata 'Author': %s\n", author);
    } else {
        printf("Failed to get metadata 'Author'.\n");
    }

    // List all metadata
    unsigned int count;
    char** metadata_keys = f3d_image_all_metadata(img, &count);
    if (metadata_keys) {
        printf("Metadata keys (%u):\n", count);
        for (unsigned int i = 0; i < count; ++i) {
            printf("  %s\n", metadata_keys[i]);
            free(metadata_keys[i]);
        }
        free(metadata_keys);
    } else {
        printf("Failed to list metadata keys.\n");
    }

    // Create a reference image for comparison
    f3d_image_t* ref_img = f3d_image_new();
    if (ref_img) {
        f3d_image_set_resolution(ref_img, 800, 600);
        f3d_image_set_content(ref_img, buffer);

        // Compare images
        double error;
        f3d_image_t* diff_img = f3d_image_new();
        int result = f3d_image_compare(img, ref_img, 0.0, diff_img, &error);
        if (result == 0) {
            printf("Images are identical.\n");
        } else {
            printf("Images differ with error: %f\n", error);
        }

        f3d_image_delete(diff_img);
        f3d_image_delete(ref_img);
    } else {
        printf("Failed to create reference image.\n");
    }

    // Clean up
    free(buffer);
    f3d_image_delete(img);

    return 0;
}
