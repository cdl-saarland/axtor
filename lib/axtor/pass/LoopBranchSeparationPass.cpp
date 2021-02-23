/*
 * Duplicator.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/pass/LoopBranchSeparationPass.h>
#include <axtor/util/llvmDebug.h>

#include <axtor/util/llvmShortCuts.h>

using namespace llvm;

namespace axtor {

char LoopBranchSeparationPass::ID = 0;

llvm::RegisterPass<LoopBranchSeparationPass> __regLBSep("loopbranchseparator", "axtor - detaches break/continue branches from n-way conditionals",
					true,
                    false);

	LoopBranchSeparationPass::LoopBranchSeparationPass() : llvm::ModulePass(ID) {}

	void LoopBranchSeparationPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		usage.addRequired<llvm::LoopInfoWrapperPass>();
		usage.addRequired<TargetProvider>();
	}

	bool LoopBranchSeparationPass::runOnModule(llvm::Module & mod)
	{
#ifdef DEBUG_PASSRUN
		std::cerr << "\n\n##### PASS: Loop Branch Separation #####\n\n";
		std::cerr << "### BEFORE loop branch separation: \n";
		llvm::errs() << mod;
#endif
		bool changed = false;

		for (llvm::Module::iterator func = mod.begin(); func != mod.end() ; ++func)
			if (! func->empty())
				changed |= runOnFunction(*func);

#ifdef DEBUG
	std::cerr << "### AFTER loop branch separation: \n";
	llvm::errs() << mod;
	verifyModule(mod);
#endif

		return changed;
	}

	llvm::BasicBlock * LoopBranchSeparationPass::breakSpecialEdge(llvm::Function * func, llvm::BasicBlock * srcBlock, llvm::BasicBlock * targetBlock, BasicBlock * insertBefore)
	{
		llvm::LLVMContext & context = SharedContext::get();

		//break branch
		llvm::BasicBlock * detachBlock = llvm::BasicBlock::Create(context, "detached", func, insertBefore);
		llvm::BranchInst::Create(targetBlock, detachBlock);

		//adapt PHI-nodes
		for(llvm::BasicBlock::iterator inst = targetBlock->begin(); llvm::isa<llvm::PHINode>(inst); inst++)
		{
			llvm::PHINode * phi = llvm::cast<llvm::PHINode>(inst);
			int idx = phi->getBasicBlockIndex(srcBlock);
			if (idx > -1) {
				phi->setIncomingBlock(idx, detachBlock);
			}
		}

		return detachBlock;
	}

	bool LoopBranchSeparationPass::runOnFunction(llvm::Function & func)
	{
		bool changed = false;

		llvm::LoopInfo & loopInfo = getAnalysis<llvm::LoopInfoWrapperPass>(func).getLoopInfo();

		llvm::Function::iterator next;
		for(llvm::Function::iterator block = func.begin(); block != func.end(); block = next)
		{
			if (next != func.end()) {
				next = block;
				next++;
			}

			// check all branches of n-way terminators (n > 1)
			llvm::Instruction * termInst = block->getTerminator();
			llvm::Loop * loop = loopInfo.getLoopFor(&*block);

			if (loop && termInst->getNumSuccessors() > 1)
			{
				llvm::BasicBlock * cntBlock = loop->getHeader();
				llvm::BasicBlock * breakBlock = loop->getExitBlock();

				for(uint i = 0; i < termInst->getNumSuccessors(); i++)
				{
					llvm::BasicBlock * specialBlock = NULL;
					llvm::BasicBlock * branchBlock = termInst->getSuccessor(i);

					//if the branch goes to the loop header or the loop exit break the edge
					if (branchBlock == cntBlock)
						specialBlock = cntBlock;
					else if (branchBlock == breakBlock)
						specialBlock = breakBlock;

					if (specialBlock)
					{
#ifdef DEBUG
						std::string name = block->getName().str();
						std::cerr << "#\tbreak special loop branch at block :" << name << std::endl;
#endif
						llvm::BasicBlock * detachBlock = breakSpecialEdge(&func, &*block, specialBlock, cast<BasicBlock>(next));

						termInst->setSuccessor(i, detachBlock);
						changed = true;
					}
				}
			}
		}

		return changed;
	}

        llvm::StringRef LoopBranchSeparationPass::getPassName() const
	{
		return "axtor - loop branch separation pass";
	}

}
