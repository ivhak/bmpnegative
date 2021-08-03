typedef struct {
    unsigned char g, b, r;
} pixel;

__kernel void negative_kernel(__global pixel *rawdata_in,
                              __global pixel *rawdata_out,
                              int width,
                              int height)
{
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);

    if (x >= width || y >= height)  return;

    rawdata_out[y*width+x].r = 255 - rawdata_in[y*width+x].r;
    rawdata_out[y*width+x].g = 255 - rawdata_in[y*width+x].g;
    rawdata_out[y*width+x].b = 255 - rawdata_in[y*width+x].b;

}
