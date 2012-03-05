#!/bin/bash

LLVM_REVISION=`svn info ../.. | grep Revision | sed 's/Revision: //'`
LLVM_PATCH="patches/llvm_opencl_patch_r${LLVM_REVISION}.patch"
CLANG_REVISION=`svn info ../../tools/clang | grep Revision | sed 's/Revision: //'`
CLANG_PATCH="patches/clang_opencl_patch_r${CLANG_REVISION}.patch"

mkdir -p patches

echo -n "Regenerating Clang patch file (r${CLANG_REVISION}) .."
svn diff ../../tools/clang > ${CLANG_PATCH}
echo "DONE"

# echo -n "Regenerating LLVM patch file  (r${LLVM_REVISION}) .."
# svn diff ../.. > ${LLVM_PATCH}
# echo "DONE"
