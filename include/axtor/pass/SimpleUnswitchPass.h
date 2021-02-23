/*
 * SimpleUnswitchPass.h
 *
 *  Created on: 16.03.2011
 *      Author: Simon Moll
 */


#ifndef SIMPLEUNSWITCHPASS_HPP_
#define SIMPLEUNSWITCHPASS_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/BasicBlock.h>

namespace axtor {
	/*
	 * replaces Switch- terminators with a cascade of 2-way branches
	 */
	class SimpleUnswitchPass : public llvm::ModulePass
	{
		bool runOnFunction(llvm::Function * func);

		void processSwitch(llvm::Function * func, llvm::BasicBlock * switchBlock);

	public:
		static char ID;

		virtual llvm::StringRef getPassName() const;

		virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

		SimpleUnswitchPass() :
			llvm::ModulePass(ID)
		{}

		virtual ~SimpleUnswitchPass();

		virtual bool runOnModule(llvm::Module& M);
	};

}

#endif /* SIMPLEUNSWITCHPASS_HPP_ */
