##===- projects/sample/lib/sample/Makefile -----------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../..

#
# Give the name of a library.  This will build a dynamic version.
#

ENABLE_PACKETIZER=0

LIBRARYNAME=axtorOpenCLDriver
SHARED_LIBRARY=1
LINK_LIBS_IN_SHARED=1

ifndef ENABLE_PACKTIZER
SOURCES=LibraryFunctions.cpp
endif

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common



# Packetizer config
PACK_PATH="${WORKSPACE}/whole-function-vectorization"
PACK_LIB=${PACK_PATH}/lib/libWFV.a
PACK_INCLUDE=${PACK_PATH}/include

# OpenCL override
ifdef OPENCL_LIBRARY_PATH
  OPENCL_LIB=${OPENCL_LIBRARY_PATH}/libOpenCL.so
else
  OPENCL_LIB=-lOpenCL
endif

LIBS += ${OPENCL_LIB}

ifdef ENABLE_PACKETIZER
CXXFLAGS+=-I${PACK_INCLUDE}
LIBS+= ${PACK_LIB}
endif 
