##===- projects/sample/tools/sample/Makefile ---------------*- Makefile -*-===##

#
# Indicate where we are relative to the top of the source tree.
#
LEVEL=../..

#
# Give the name of the tool.
#

#
# Include Makefile.common so we know what to do.
#
include $(LEVEL)/Makefile.common

INCDIR=${PROJ_SRC_ROOT}/tools/axtorIntrinsicHeaders/
INCLUDE=-I${PROJ_INSTALL_ROOT}/include/ -I${INCDIR}

BUILDHEADER=gcc -E

headers: glsl_header ocl_header slang_api

GLSL_HPP = ${PROJ_INSTALL_ROOT}/include/axtor_glsl/GLSL_Intrinsics.h
GLSL_TMP = ${PROJ_INSTALL_ROOT}/include/axtor_glsl/GLSL_Intrinsics.tmp

OCL_HPP = ${PROJ_INSTALL_ROOT}/include/axtor_ocl/OpenCL_Intrinsics.h
OCL_TMP = ${PROJ_INSTALL_ROOT}/include/axtor_ocl/OpenCL_Intrinsics.tmp

glsl_header:
	${BUILDHEADER} ${INCLUDE} ${INCDIR}/include/Slang_Api.h -o ${GLSL_TMP}
	echo "#ifndef _AXTOR_GLSL_INTRINSICS_HPP" > ${GLSL_HPP}
	echo "#define _AXTOR_GLSL_INTRINSICS_HPP" >> ${GLSL_HPP}
	cat ${GLSL_TMP} >> ${GLSL_HPP}
	echo "#endif" >> ${GLSL_HPP}
	rm ${GLSL_TMP}

ocl_header:
	${BUILDHEADER} ${INCLUDE} ${INCDIR}/include/Compute_Api.h -o ${OCL_TMP}
	$(ECHO) "#ifndef _AXTOR_OCL_INTRINSICS_HPP" > ${OCL_HPP}
	$(ECHO) "#define _AXTOR_OCL_INTRINSICS_HPP" >> ${OCL_HPP}
	cat ${OCL_TMP} >> ${OCL_HPP}
	$(ECHO) "#endif" >> ${OCL_HPP}
	rm ${OCL_TMP}

	
install:: ocl_header glsl_header

clean::
	rm -f ${GLSL_HPP}
	rm -f ${OCL_HPP}

	