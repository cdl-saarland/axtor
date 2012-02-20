#!/bin/bash
RELDIR=../../../
source test_shader_with_glsl.sh ${RELDIR}/rtfact/shaders/src/rsl/$1.sl &> LOG_$1.txt
