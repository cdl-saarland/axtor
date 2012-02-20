#!/bin/bash

GLUEPATH=../glue/
THISDIR=$(dirname "$0")

GLUE_RUNTIME="${GLUEPATH}/deferred/glsl_glue_runtime.bc"

if [ -z "$1" ]; then
    echo >&2 "No shader specified"
    exit 1
fi

SHADER_SRC="$1"
case "$SHADER_SRC" in
    *.sl)
        echo "RenderMan Shader Source"
        COMPILER=$ANYSL_RSL_COMPILER
        BASENAME=$(basename "$SHADER_SRC" .sl)
        ;;
    *.cpp)
        echo "C++/AnySL Shader Source"
        COMPILER=$ANYSL_CXX_COMPILER
        BASENAME=$(basename "$SHADER_SRC" .cpp)
        ;;
    *)
        echo >&2 "Unknown Shader Source $1"
        exit 1
        ;;
esac

SHADER=/tmp/${BASENAME}.bc
SHADER_GPU=/tmp/${BASENAME}_gpu.bc
SHADER_GPU_OPT=bitcode/${BASENAME}_sl.bc
SHADER_GPU_LL=/tmp/${BASENAME}_sl.ll

"$COMPILER" "$SHADER_SRC" "$SHADER" || exit 1

echo "Linking $SHADER $GLUE_RUNTIME -> $SHADER_GPU"

anysl_link -o "$SHADER_GPU" "$SHADER" "$GLUE_RUNTIME" || exit 1
#rm -f "$SHADER_GPU_LL"
#llvm-dis "$SHADER_GPU"

opt -O3 $SHADER_GPU -o $SHADER_GPU_OPT

#llvm-dis $SHADER_GPU_OPT

rm -f $SHADER $SHADER_GPU 


#output 
echo "Generated shader GLSL file $SHADER_GPU"
