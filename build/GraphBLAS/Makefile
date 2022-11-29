
ROOT=../..

MK.cmake=1

include $(ROOT)/deps/readies/mk/main

#----------------------------------------------------------------------------------------------

define HELPTEXT
make build          # compile and link
  DEBUG=1             # build for debugging
  WHY=1               # explain CMake decisions (in /tmp/cmake-why)
  FORCE=1             # Force CMake rerun (default)
  CMAKE_ARGS=...      # extra arguments to CMake
  SLOW=1              # do not parallelize build (for diagnostics)
make clean         # remove build artifacts
  ALL=1              # remove entire artifacts directory

endef

#----------------------------------------------------------------------------------------------

SRCDIR=$(ROOT)/deps/GraphBLAS

CMAKE_TARGETS=graphblas_static

BINDIR=$(BINROOT)/GraphBLAS

TARGET=$(BINDIR)/libgraphblas.a

ifeq ($(OSNICK),monterey)
OSX_SDK_VER=12.0
else
OSX_SDK_VER=10.15
endif

CMAKE_FLAGS+=\
	-Wno-dev \
	-DBUILD_GRB_STATIC_LIBRARY=1 \
	-DBUILD_TESTING=off \
	-DCMAKE_POSITION_INDEPENDENT_CODE=on \
	-DCMAKE_OSX_DEPLOYMENT_TARGET=$(OSX_SDK_VER)

#----------------------------------------------------------------------------------------------

include $(MK)/defs

#----------------------------------------------------------------------------------------------

MK_CUSTOM_CLEAN=1

include $(MK)/rules

#----------------------------------------------------------------------------------------------

clean:
ifeq ($(ALL),1)
	$(SHOW)rm -rf $(BINDIR)
else
	$(SHOW)$(MAKE) -C $(BINDIR) clean
endif

.PHONY: clean
