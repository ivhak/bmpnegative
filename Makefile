ROCMPATH := /opt/rocm-4.2.0

OPENCL_DFLAGS := -DCL_TARGET_OPENCL_VERSION=220
OPENCL_IFLAGS := -I$(ROCMPATH)/opencl/include
OPENCL_LFLAGS := -lOpenCL -L$(ROCMPATH)/lib

CC      := gcc
CUDA_CC := nvcc
HIP_CC  := hipcc

CFLAGS := -O2
CXXFLAGS := -O2

OBJ := $(wildcard src/*.o)

serial:     bmpnegative-serial
hip:        bmpnegative-hip
cuda:       bmpnegative-cuda
opencl:     bmpnegative-opencl

hip-nvidia: export HIP_PLATFORM=nvidia
hip-nvidia: export CUDA_PATH=/cm/shared/apps/cuda11.0/toolkit/11.0.3
hip-nvidia: bmpnegative-hip-nvidia

bmpnegative-serial: src/bitmap.o src/bmpnegative.c
	$(CC) $(CFLAGS) $^ -o $@

bmpnegative-cuda: src/bitmap.o src/bmpnegative.cu
	$(CUDA_CC) $(CFLAGS) $^ -o $@

src/bmpnegative.hip.o: src/bmpnegative.hip.cpp
	$(HIP_CC) -c $(CXXFLAGS) $^ -o $@

bmpnegative-hip: src/bitmap.o src/bmpnegative.hip.o
	$(HIP_CC) $^ -o $@

bmpnegative-hip-nvidia: src/bitmap.o src/bmpnegative.hip.o
	$(HIP_CC) $^ -o $@

bmpnegative-opencl: src/bitmap.o src/bmpnegative.opencl.cpp
	$(CC) $(CFLAGS) $(OPENCL_DFLAGS) $(OPENCL_IFLAGS) $^ $(OPENCL_LFLAGS) -o $@

src/bitmap.o: src/bitmap.c src/bitmap.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -Rf $(OBJ) bmpnegative-*

