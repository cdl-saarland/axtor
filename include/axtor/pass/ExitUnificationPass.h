/*
 * ExitUnificationPass.h
 *
 *  Created on: 05.03.2010
 *      Author: Simon Moll
 */

#ifndef EXITUNIFICATIONPASS_HPP_
#define EXITUNIFICATIONPASS_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/SmallVector.h>

#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/llvmConstant.h>
#include <axtor/CommonTypes.h>
#include <axtor/pass/CNSPass.h>

namespace axtor {

/*
 * this pass enumerates all loop exits to the parent loop and pulls them out into a single branching exit block
 */
class ExitUnificationPass : public llvm::ModulePass
{

	/*
	 * generate a switch-like construct branching to elements of dest on their index
	 */
	void appendEnumerationSwitch(llvm::Value * val, std::vector<llvm::BasicBlock*> dests, llvm::BasicBlock * block);

	/*
	 * if this loop has multiple exits to the parent loop enumerate them and move the branches to a dedicated exit block
	 *
	 * @return true, if the loop has changed
	 */
	bool unifyLoopExits(llvm::Function & func, llvm::Loop * loop);

	bool runOnFunction(llvm::Function & func);

public:
	static char ID;

	ExitUnificationPass() : llvm::ModulePass(ID) {}

	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

	bool runOnModule(llvm::Module & M);

        llvm::StringRef getPassName() const;
};

}



#endif /* EXITUNIFICATIONPASS_HPP_ */
