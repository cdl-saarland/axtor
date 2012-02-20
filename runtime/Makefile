INCLUDE=-Iinclude/ -I../intrinsics
BUILDHEADER=gcc -E

headers: glsl_header ocl_header slang_api

GLSL_HPP = ${PREFIX}/include/axtor/GLSL/GLSL_Intrinsics.h
GLSL_TMP = ${PREFIX}/include/axtor/GLSL/GLSL_Intrinsics.tmp

OCL_HPP = ${PREFIX}/include/axtor/OCL/OpenCL_Intrinsics.h
OCL_TMP = ${PREFIX}/include/axtor/OCL/OpenCL_Intrinsics.tmp

slang_api:
	mkdir -p ${PREFIX}/include/axtor/runtime
	cp ../intrinsics/GLSL_Intrinsics.def ${PREFIX}/include/axtor/runtime
	cp -a include/*.h ${PREFIX}/include/axtor/runtime

glsl_header: ../intrinsics/GLSL_Intrinsics.def
	mkdir -p ${PREFIX}/include/axtor/GLSL
	${BUILDHEADER} ${INCLUDE} include/Slang_Api.h -o ${GLSL_TMP}
	echo "#ifndef _AXTOR_GLSL_INTRINSICS_HPP" > ${GLSL_HPP}
	echo "#define _AXTOR_GLSL_INTRINSICS_HPP" >> ${GLSL_HPP}
	cat ${GLSL_TMP} >> ${GLSL_HPP}
	echo "#endif" >> ${GLSL_HPP}
	rm ${GLSL_TMP}

ocl_header: ../intrinsics/OCL_Intrinsics.def
	mkdir -p ${PREFIX}/include/axtor/OCL
	${BUILDHEADER} ${INCLUDE} include/Compute_Api.h -o ${OCL_TMP}
	echo "#ifndef _AXTOR_OCL_INTRINSICS_HPP" > ${OCL_HPP}
	echo "#define _AXTOR_OCL_INTRINSICS_HPP" >> ${OCL_HPP}
	cat ${OCL_TMP} >> ${OCL_HPP}
	echo "#endif" >> ${OCL_HPP}
	rm ${OCL_TMP}
	cp ${OCL_HPP} ../frontend_wrapper/.
