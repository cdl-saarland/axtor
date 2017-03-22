/*
 * LOOPBRANCHSEPARATIONPASS.h
 *
 *  Created on: 19.02.2011
 *      Author: Simon Moll
 */

#ifndef LOOPBRANCHSEPARATIONPASS_HPP_
#define LOOPBRANCHSEPARATIONPASS_HPP_

#include <axtor/config.h>

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <axtor/console/CompilerLog.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/llvmDomination.h>
#include <axtor/CommonTypes.h>
#include <axtor/pass/TargetProvider.h>



namespace axtor {

/*
 * Detaches loop branches from 2-way conditionals to simplify the AST-extraction
 */
class LoopBranchSeparationPass : public llvm::ModulePass
{
	/*
	 * breaks the edge and inserts adapts the PHI-nodes
	 */
	llvm::BasicBlock * breakSpecialEdge(llvm::Function * func, llvm::BasicBlock * srcBlock, llvm::BasicBlock * targetBlock, llvm::BasicBlock * insertBefore);

	bool runOnFunction(llvm::Function & func);
public:
	static char ID;

	LoopBranchSeparationPass();

	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

	bool runOnModule(llvm::Module & mod);

        llvm::StringRef getPassName() const;
};

}


#endif /* LOOPBRANCHSEPARATIONPASS_HPP_ */
