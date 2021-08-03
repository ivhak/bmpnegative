#include "CL/cl.h"
#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "bitmap.h"
}

#define BLOCK_X 16
#define BLOCK_Y 16
#define MAX_SOURCE_SIZE (0x100000)

#define DIE_IF_CL(err_code, str) do { if (err != CL_SUCCESS) {fprintf(stderr,"%d: %s\n", err_code, str); exit(1); } } while (0)

char *load_kernel_source(const char *filename);

int main(int argc, char **argv) {

    char *filename_in  = argv[1];
    char *filename_out = argv[2];

    image_t *in = image_from_filename(filename_in);
    image_t *out = new_image(in->width, in->height);

    // OpenCL source can be placed in the source code as text strings or read from another file.
    char *source_str = load_kernel_source("./src/kernel.cl");

    const int size_of_image = in->width*in->height*sizeof(pixel);

    // Setup OpenCL
    cl_platform_id    platform_id;
    cl_device_id      device_id;
    cl_context        context;
    cl_command_queue  queue;
    cl_program        program;
    cl_kernel         kernel;

    cl_mem d_rawdata_in, d_rawdata_out;

    cl_int err;

    err = clGetPlatformIDs(1, &platform_id, NULL);
    DIE_IF_CL(err, "Could not get platforms.");

    err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    DIE_IF_CL(err, "Could not get device.");

    // Create a compute context with the GPU
    context = clCreateContextFromType(NULL, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
    DIE_IF_CL(err, "Could not create compute context.");

    // Create a command queue
    queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &err);
    DIE_IF_CL(err, "Could not create command queue.");

    // Create the compute program
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, NULL, &err);
    DIE_IF_CL(err, "Could not create kernel program.");
    free(source_str);

    // Build the compute program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err == CL_BUILD_PROGRAM_FAILURE) {
        size_t log_size;
        char *log;

        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("%s\n", log);
    }
    DIE_IF_CL(err, "Could not build kernel program.");


    // Allocate the buffer memory objects aka device side buffers
    d_rawdata_in  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size_of_image, (void*)in->rawdata, &err);
    DIE_IF_CL(err, "Could not create in buffer.");

    d_rawdata_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size_of_image, NULL, &err);
    DIE_IF_CL(err, "Could not create out buffer.");

    kernel = clCreateKernel(program, "negative_kernel", &err);
    DIE_IF_CL(err, "Could not create kernel");

    // Set the args
    err  = clSetKernelArg(kernel, 0,   sizeof(cl_mem), &d_rawdata_in);
    err |= clSetKernelArg(kernel, 1,   sizeof(cl_mem), &d_rawdata_out);
    err |= clSetKernelArg(kernel, 2,   sizeof(int),    &in->width);
    err |= clSetKernelArg(kernel, 3,   sizeof(int),    &in->height);
    DIE_IF_CL(err, "Failed to set kernel argument.");


    // Create N-D range object with work-item dimensions and execute kernel
    // Round up the image width and height to the nearest multiple of BLOCK_X
    // and BLOCK_Y respectively.
    size_t global_work_size[2];
    global_work_size[0] = BLOCK_X*((in->width  + BLOCK_X - 1)/BLOCK_X);
    global_work_size[1] = BLOCK_Y*((in->height + BLOCK_Y - 1)/BLOCK_Y);

    size_t local_work_size[2] = {BLOCK_X, BLOCK_Y};

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    DIE_IF_CL(err, "Failed to run kernel.");

    // Copy back from the device-side array
    err = clEnqueueReadBuffer(queue, d_rawdata_out, CL_TRUE, 0, size_of_image, out->rawdata, 0, NULL, NULL);
    DIE_IF_CL(err, "Failed to copy back to device.");

    clFinish(queue);

    // Write the image back to disk
    save_image(out, filename_out);

    clReleaseKernel(kernel);
    clReleaseMemObject(d_rawdata_in);
    clReleaseMemObject(d_rawdata_out);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

};


char *load_kernel_source(const char *filename)
{
    // OpenCL source can be placed in the source code as text strings or read from another file.
    FILE *fp;
    size_t source_size;
    char *source_str;

    fp = fopen(filename, "r");
    if (!fp) {
       perror("fopen");
       exit(1);
    }

    source_str = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp );
    fclose(fp);
    return source_str;
}
