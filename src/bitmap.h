#ifndef BITMAP_H
#define BITMAP_H

// NOTE: The struct members are in the "wrong" order as  bitmap images are
// stored "backwards" of what one might assume, i.e., the first pixel after the
// header in a bitmap file is in bottom right corner, rather than the top left.
typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
} pixel;

typedef struct {
    unsigned int width;
    unsigned int height;
    pixel *rawdata;
    pixel **data;
} image_t;


image_t *new_image(unsigned int const width, unsigned int const height);
void free_image(image_t *image);
int  load_image(image_t *image, char const *filename);
int  save_image(image_t *image, char const *filename);

image_t *image_from_filename(const char *filename);

#endif
