if [ -z ${LLVM_PREFIX} ]; then
  echo "oclang.sh was not correctly installed. Aborting .."
  exit;
fi 

LLVM_PATH=${LLVM_PREFIX}/bin
LLVM_INCLUDE=${LLVM_PREFIX}/include

CLANG=${LLVM_PATH}/clang
LLVMAS=${LLVM_PATH}/llvm-as
OCLDEF="${LLVM_PREFIX}/include/axtor_ocl/ocldef.h"

$CLANG -ccc-host-triple ptx32 -include ${OCLDEF} -emit-llvm -fno-builtin -c ${@:1}
