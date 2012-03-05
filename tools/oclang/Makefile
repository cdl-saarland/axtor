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

SRC_PATH=${PROJ_SRC_ROOT}/tools/oclang/
OBJ_PATH=${PROJ_OBJ_ROOT}/
BIN_PATH=${PROJ_INSTALL_ROOT}/bin/

OCLANG_TMP=${PROJ_OBJ_ROOT}/oclang.sh


install:: ${SRC_PATH}/oclang.sh
	$(ECHO) "# !/bin/bash" > ${OCLANG_TMP} 
	$(ECHO) "LLVM_PREFIX=\"${PROJ_INSTALL_ROOT}\"" >> ${OCLANG_TMP}
	cat ${SRC_PATH}/oclang.sh >> ${OCLANG_TMP}
	cp ${OCLANG_TMP} ${BIN_PATH} 
	chmod +x ${BIN_PATH}/oclang.sh
	
clean:: 
	rm -f ${BIN_PATH}/oclang.sh