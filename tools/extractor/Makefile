##===- projects/sample/tools/sample/Makefile ---------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../..

#
# Give the name of the tool.
#
TOOLNAME=axtor

#
# List libraries that we'll need
# We use LIBS because sample is a dynamic library.
#
# LINK_COMPONENTS = all
LINK_COMPONENTS = core object scalaropts support transformutils ipo instrumentation bitreader bitwriter asmparser
# vectorize

USEDLIBS=Axtor_OCL.a axtorInterface.a axtorPass.a axtorParsers.a axtorSolvers.a axtorAST.a axtorWriter.a \
	axtorMetainfo.a axtorConsole.a axtorCNS.a axtorIntrinsics.a axtorUtil.a
	
#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

# LIBS += -lAxtor_OCL -lAxtor
CPPFLAGS += -DENABLE_OPENCL

