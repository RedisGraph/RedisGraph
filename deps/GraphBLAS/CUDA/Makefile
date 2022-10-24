#-------------------------------------------------------------------------------
# GraphBLAS/CUDA/Makefile
#-------------------------------------------------------------------------------

# cuda 10.1+ is assumed

all: library

GXX     ?= g++
DOXYGEN ?= doxygen
CXXFLAGS ?= -O3 -Wall -g -fmessage-length=80

CXX11 ?= 1

CUDA_DIR ?= /usr/local/cuda

CXXFLAGS += -pthread

ifeq ($(CXX11),1)
	CXXFLAGS += -std=c++14
endif

EMBED_BEGIN = -rdynamic -Wl,-b,binary,
EMBED_END   = ,-b,default

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CXXFLAGS += -D LINUX
	CUDA_LIB_DIR = $(CUDA_DIR)/lib64
else ifeq ($(UNAME_S),Darwin)
	CUDA_LIB_DIR = $(CUDA_DIR)/lib
endif

INC += -I$(CUDA_DIR)/include
LIB += -ldl -L$(CUDA_LIB_DIR) -lcuda -lcudart -lnvrtc


GCC ?= gcc

SRC = GB*.cu
SRC2 = $(notdir $(wildcard $(SRC)))
OBJ = $(SRC2:.cu=.o)
cSRC = $(wildcard *.c) 
cOBJ = $(cSRC:.c=.o)
cppSRC = $(wildcard *.cpp)
cppOBJ = $(cppSRC:.cpp=.o) 

I = -I. -I../Source -I../Source/Template -I../Include -I../rmm/rmm/include/  -Irmm/thirdparty/spdlog/include 
SO_NAME = libgraphblascuda.so
SO_OPTS = --shared \
    -Xlinker -soname \
    -Xlinker $(SO_NAME)

LIBS = -L/usr/local/cuda/lib64 -lcudadevrt -lcudart 

CUDA_OPTS = -O2 --cudart=shared --gpu-architecture=compute_75 \
        --relocatable-device-code true \
        --std=c++14 -Xcompiler -fPIC

library: $(SO_NAME) 

HEADERS = jitify.hpp 

TEMPLATES :=  $(wildcard templates/*.cu)

JIT_TEMP := $(patsubst %.cu, %.cu.jit, $(TEMPLATES))

%.cu: %.cutmp
	cp $? $@

%.cu.jit: %.cu 
	./stringify $? > $@

stringify: stringify.cpp
	$(GXX) -o $@ $< -O3 -Wall

    
doc: jitify.hpp Doxyfile
	$(DOXYGEN) Doxyfile
.PHONY: doc

test: $(cOBJ)
	@echo $(cOBJ)

$(cppOBJ): %.o: %.cpp 
	$(GXX) $(I) -fPIC -o $@ -c $< -O2 -Wall

$(cOBJ): %.o: %.c 
	$(GCC) $(I) -fPIC -o $@ -c $< -O2 -Wall

$(SO_NAME): $(OBJ) $(cOBJ) $(cppOBJ) $(JIT_TEMP) GB_AxB_dot3_cuda.o
	echo $(OBJ)
	nvcc $(SO_OPTS) $(LIBS) $(OBJ) $(cOBJ) -o $@

GB_AxB_dot3_cuda.o: $(JIT_TEMP) GB_cuda_semiring_factory.hpp matrix.h
%.o: %.cu
	nvcc -c $(I) $(CUDA_OPTS) -o $@ $< $(LIBS)


config:
	nvidia-smi
	nvcc --version
	@echo " "
	@echo "SO_NAME:   " $(SO_NAME)
	@echo "SO_OPTS:   " $(SO_OPTS)
	@echo "LIBS:      " $(LIBS)
	@echo "CUDA_OPTS: " $(CUDA_OPTS)
	@echo "SRC:       " $(SRC)
	@echo "OBJ:       " $(OBJ)
	@echo "I:         " $(I)
	@echo " "
	gcc  --version
	icc  --version

clean:
	rm -f *.o
	rm -f stringify
.PHONY: clean

distclean: clean
	rm -f *.so *.a

purge: distclean

################################################################################


EMBED_BEGIN = -rdynamic -Wl,-b,binary,
EMBED_END   = ,-b,default

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CXXFLAGS += -D LINUX
	CUDA_LIB_DIR = $(CUDA_DIR)/lib64
else ifeq ($(UNAME_S),Darwin)
	CUDA_LIB_DIR = $(CUDA_DIR)/lib
endif




