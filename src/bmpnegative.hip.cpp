#include "hip/hip_runtime.h"
#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "bitmap.h"
}

#define HIP_CHECK(command) {     \
    hipError_t status = command; \
    if (status!=hipSuccess) {     \
        printf("(%s:%d) Error: Hip reports %s\n", __FILE__, __LINE__, hipGetErrorString(status)); \
        exit(1); \
    } \
}

__global__ void negative_kernel(pixel *rawdata_in, pixel *rawdata_out, int width, int height)
{
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    rawdata_out[y*width+x].r = 255 - rawdata_in[y*width+x].r;
    rawdata_out[y*width+x].g = 255 - rawdata_in[y*width+x].g;
    rawdata_out[y*width+x].b = 255 - rawdata_in[y*width+x].b;
}

int main(int argc, char *argv[]) {

    char *filename_in  = argv[1];
    char *filename_out = argv[2];

    image_t *in = image_from_filename(filename_in);
    image_t *out = new_image(in->width, in->height);

    const int width  = in->width;
    const int height = in->height;

    const int size_of_all_pixels = width*height*sizeof(pixel);

    pixel *d_rawdata_in, *d_rawdata_out;

    HIP_CHECK(hipMalloc((void **)&d_rawdata_in,  size_of_all_pixels));
    HIP_CHECK(hipMalloc((void **)&d_rawdata_out, size_of_all_pixels));

    HIP_CHECK(hipMemcpy(d_rawdata_in, in->rawdata, size_of_all_pixels, hipMemcpyHostToDevice));

    dim3 block_size(32, 32);
    dim3 grid_size((width  + block_size.x - 1) / block_size.x,
                   (height + block_size.y - 1) / block_size.y);

    hipLaunchKernelGGL(negative_kernel, dim3(grid_size), dim3(block_size),
                       0, 0, d_rawdata_in, d_rawdata_out, width, height);


    HIP_CHECK(hipMemcpy(out->rawdata, d_rawdata_out, size_of_all_pixels, hipMemcpyDeviceToHost));

    save_image(out, filename_out);
}
