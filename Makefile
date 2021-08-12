ROCM_PATH ?= /opt/rocm

OPENCL_DFLAGS := -DCL_TARGET_OPENCL_VERSION=220
OPENCL_IFLAGS := -I$(ROCM_PATH)/opencl/include
OPENCL_LFLAGS := -lOpenCL -L$(ROCM_PATH)/lib

CC      := gcc
CUDA_CC := nvcc
HIP_CC  := hipcc

CFLAGS := -O2
CXXFLAGS := -O2

OBJ := $(wildcard src/*.o)

serial:     bmpnegative-serial
cuda:       bmpnegative-cuda

opencl:     bmpnegative-opencl

hip-amd:    bmpnegative-hip-amd
hip-nvidia: bmpnegative-hip-nvidia

all: serial hip-amd hip-nvidia cuda opencl

bmpnegative-serial: src/bitmap.o src/bmpnegative.c
	$(CC) $(CFLAGS) $^ -o $@

bmpnegative-cuda: src/bitmap.o src/bmpnegative.cu
	$(CUDA_CC) $(CFLAGS) $^ -o $@

src/bmpnegative-%.hip.o: src/bmpnegative.hip.cpp
	HIP_PLATFORM=$* $(HIP_CC) -c $(CXXFLAGS) $^ -o $@

bmpnegative-hip-%: src/bitmap.o src/bmpnegative-%.hip.o
	HIP_PLATFORM=$* $(HIP_CC) $^ -o $@

bmpnegative-opencl: src/bitmap.o src/bmpnegative.opencl.cpp
	$(CC) $(CFLAGS) $(OPENCL_DFLAGS) $(OPENCL_IFLAGS) $^ $(OPENCL_LFLAGS) -o $@

src/bitmap.o: src/bitmap.c src/bitmap.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -Rf $(OBJ) bmpnegative-*

