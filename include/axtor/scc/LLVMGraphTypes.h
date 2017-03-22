/*
 * LLVMGraphTypes.h
 *
 *  Created on: 19.02.2010
 *      Author: Simon Moll
 */

#ifndef LLVMGRAPHTYPES_HPP_
#define LLVMGRAPHTYPES_HPP_

#include <llvm/BasicBlock.h>
#include "GraphTypes.h"
#include "Tarjan.h"

typedef DirectedGraph<llvm::BasicBlock> FunctionGraph;
typedef Tarjan<llvm::BasicBlock> LLVMTarjan;

#endif /* LLVMGRAPHTYPES_HPP_ */
