OPENCL_DFLAGS := -DCL_TARGET_OPENCL_VERSION=220
OPENCL_IFLAGS := -I/opt/rocm-4.2.0/opencl/include
OPENCL_LFLAGS := -lOpenCL -L/opt/rocm-4.2.0/lib

CC      := gcc
CUDA_CC := nvcc
HIP_CC  := hipcc

OBJ := $(wildcard src/*.o)

serial: bmpnegative-serial
hip:    bmpnegative-hip
cuda:   bmpnegative-cuda
opencl: bmpnegative-opencl

bmpnegative-serial: src/bitmap.o src/bmpnegative.c
	$(CC) $(CFLAGS) $^ -o $@

bmpnegative-cuda: src/bitmap.o src/bmpnegative.cu
	$(CUDA_CC) $(CFLAGS) $^ -o $@

bmpnegative-hip: src/bitmap.o src/bmpnegative.hip.cpp
	$(HIP_CC) $(CFLAGS) $^ -o $@

src/bmpnegative.opencl.o: src/bmpnegative.opencl.cpp
	$(CC) -c $(CFLAGS) $(OPENCL_DFLAGS) $(OPENCL_IFLAGS) $^ -o $@

bmpnegative-opencl: src/bitmap.o src/bmpnegative.opencl.o
	$(CC) $^ $(OPENCL_LFLAGS) -o $@

src/bitmap.o: src/bitmap.c src/bitmap.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -Rf $(OBJ) bmpnegative-*

