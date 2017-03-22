/*
 * OpaqueTypeRenamer.h
 *
 *  Created on: 09.05.2010
 *      Author: Simon Moll
 */

#ifndef OPAQUETYPERENAMER_HPP_
#define OPAQUETYPERENAMER_HPP_

#include <axtor/config.h>

#include <iostream>

#include <llvm/IR/LLVMContext.h>
//#include <llvm/TypeSymbolTable.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>

#include <axtor/util/llvmShortCuts.h>

/*
 * removes the "class." or "struct." suffixes from types in the TypeSymbolTable
 */
namespace axtor
{

class OpaqueTypeRenamer : public llvm::ModulePass
{
	/*
	 * replace all calls to alloca_*() functions to AllocaInsts with the same type
	 */
	//void replaceAllocaIntrinsics(llvm::Module & mod);

public:
	static char ID;
	OpaqueTypeRenamer();

	virtual bool runOnModule(llvm::Module & mod);
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
};

}

#endif /* OPAQUETYPERENAMER_HPP_ */
