/*
 * TrivialReturnSplitter.cpp
 *
 *  Created on: 08.02.2011
 *      Author: gnarf
 */

#include <axtor/pass/TrivialReturnSplitter.h>
#include <axtor/util/llvmDuplication.h>

#include <axtor/config.h>

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Analysis/Passes.h>

#include <axtor/util/stringutil.h>
#include <axtor/util/llvmShortCuts.h>

#include <axtor/writer/SyntaxWriter.h>
#include <axtor/CommonTypes.h>

#include <axtor/pass/ExitUnificationPass.h>

namespace axtor {
	llvm::RegisterPass<axtor::TrivialReturnSplitter> __regReturnSplitter("returnsplitter", "axtor - trivial return splitter",
		true,
		false);

	char TrivialReturnSplitter::ID = 0;

	bool TrivialReturnSplitter::runOnFunction(llvm::Function & F) {
		for(auto & block : F) {
			if (llvm::isa<llvm::ReturnInst>(block.begin())) {
				splitNode(&block);
				return true;
			}
		}
		return false;
	}

	void TrivialReturnSplitter::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
	}

	const char * TrivialReturnSplitter::getPassName() const
	{
		return "axtor - trivial returning block splitter";
	}

	TrivialReturnSplitter::~TrivialReturnSplitter()
	{
	}
}
