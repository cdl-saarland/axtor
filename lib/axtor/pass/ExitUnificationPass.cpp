/*
 * ExitUnificationPass.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/pass/ExitUnificationPass.h>
#include <axtor/util/llvmDebug.h>
#include <axtor/util/llvmTools.h>

using namespace llvm;

namespace axtor {

llvm::RegisterPass<ExitUnificationPass> __regExitUnificationPass("loopexitenum", "axtor - loop exit enumeration pass",
					false,
                    false);

char ExitUnificationPass::ID = 0;

	/*
	 * generate a switch-like construct branching to elements of dest on their index
	 */
	void ExitUnificationPass::appendEnumerationSwitch(llvm::Value * val, std::vector<llvm::BasicBlock*> dests, llvm::BasicBlock * block)
	{
		assert(! dests.empty() && block && val);
		llvm::SwitchInst * switchInst = llvm::SwitchInst::Create(val, dests.front(), dests.size(), block);

		uint exitID = 0;
		for(BlockVector::iterator it = dests.begin(); it != dests.end(); ++it, ++exitID)
		{
			llvm::BasicBlock * dest = *it;
			switchInst->addCase(get_uint(exitID), dest);
		}
	}

	/*
	 * if this loop has multiple exits to the parent loop enumerate them and move the branches to a dedicated exit block
	 *
	 * @return true, if the loop has changed
	 */
	bool ExitUnificationPass::unifyLoopExits(llvm::Function & func, llvm::Loop * loop)
	{
		bool changed = false;
		for (llvm::Loop::iterator itSubLoop = loop->begin(); itSubLoop != loop->end(); ++itSubLoop) {
			changed |= unifyLoopExits(func, *itSubLoop);
		}

		llvm::Loop * parent = loop->getParentLoop();

		llvm::errs() << *loop;

		if (loop->getLoopDepth() <= 1) {
			std::cerr << "LEE: outer-most multi exit loop\n";
			return changed; // the exits will not reach this loop again, so we do not care
		}


		BlockSet exitBlocks;
		getUniqueExitBlocks(*loop, exitBlocks);
		if (exitBlocks.size() <= 1) {
			std::cerr << "LEE: loop has at most one exit block\n";
			return changed;
		} else {
			std::cerr << "LEE: inner multi exit..\n";
		}

		BlockPairVector edges;
		getExitEdges(*loop, edges);

		llvm::Type * intType = llvm::IntegerType::get(func.getContext(), 32);

		llvm::BasicBlock * uniqueExitBlock = llvm::BasicBlock::Create(func.getContext(), "exitswitch", &func, edges.begin()->second);
		parent->addBlockEntry(uniqueExitBlock);
    // FIXME
		llvm::PHINode * phi = llvm::PHINode::Create(intType, 0, "exitID", uniqueExitBlock);
 		std::vector<llvm::BasicBlock*> enumeratedExits;

		for (BlockPairVector::iterator edge = edges.begin(); edge != edges.end(); ++edge)
		{
			llvm::BasicBlock * from = edge->first;
			llvm::BasicBlock * to = edge->second;

			//only enumerate exits to the parent loop
			if (parent->contains(to))
			{
				auto term = from->getTerminator();

				for(uint succIdx = 0; succIdx < term->getNumSuccessors(); ++succIdx)
				{
					llvm::BasicBlock * target = term->getSuccessor(succIdx);

					if (target == to)
					{
						uint exitID = enumeratedExits.size();

						llvm::Constant * exitConstant = get_uint(exitID);
						phi->addIncoming(exitConstant, from);

						term->setSuccessor(succIdx, uniqueExitBlock);
						std::cerr << "LEE: exiting from " << from->getName().str() << " to " << to->getName().str() << " id " << exitID << "\n";
						enumeratedExits.push_back(to);
					}
				}
			}
		}

		appendEnumerationSwitch(phi, enumeratedExits, uniqueExitBlock);

		return true;
	}


	void ExitUnificationPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		usage.addRequired<llvm::LoopInfoWrapperPass>();
	}


	bool ExitUnificationPass::runOnFunction(llvm::Function & func)
	{
#ifdef DEBUG_LEU
		std::cerr << "### Function " << func.getName().str() << " before LEU ###\n";
		llvm::errs() << func;
#endif
		llvm::LoopInfo & loopInfo = getAnalysis<llvm::LoopInfoWrapperPass>(func).getLoopInfo();

		for(llvm::LoopInfo::iterator loop = loopInfo.begin(); loop != loopInfo.end(); ++loop)
		{
			unifyLoopExits(func, *loop);
		}

#ifdef DEBUG_LEU
		std::cerr << "[EOF]\n";
#endif

		return false;
	}

	bool ExitUnificationPass::runOnModule(llvm::Module & M)
	{
#ifdef DEBUG_PASSRUN
		std::cerr << "\n\n##### PASS: Loop Exit Enumeration #####\n\n";
#endif

		bool changed = false;
		for (llvm::Module::iterator func = M.begin(); func != M.end(); ++func)
		{
			if (! func->isDeclaration())
				changed |= runOnFunction(*func);
		}

#ifdef DEBUG
		std::cerr << "\n\n### Module after LEE #####\n\n";
		writeModuleToFile(&M, "Lee_dump.bc");
		llvm::errs() << M;
		verifyModule(M);
#endif

		return changed;
	}

        llvm::StringRef ExitUnificationPass::getPassName() const	{
		return "axtor - loop exit enumeration";
	}
}
