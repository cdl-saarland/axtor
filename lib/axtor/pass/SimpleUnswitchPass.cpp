/*
 * SimpleUnswitchPass.cpp
 *
 *  Created on: 16.03.2011
 *      Author: gnarf
 */

#include <axtor/pass/SimpleUnswitchPass.h>
#include <llvm/IR/Constants.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

#include <axtor/util/stringutil.h>

namespace axtor {


	char SimpleUnswitchPass::ID = 0;

	namespace {
		llvm::RegisterPass<SimpleUnswitchPass> __regUnswitchPass("unswitch", "axtor - simple unswitch pass",
							true /* mutates the CFG */,
							false /* transformation */);
	}

	SimpleUnswitchPass::~SimpleUnswitchPass() {}

        llvm::StringRef SimpleUnswitchPass::getPassName() const
	{
		return "unswitch";
	}

	void SimpleUnswitchPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{}

	void SimpleUnswitchPass::processSwitch(llvm::Function * func, llvm::BasicBlock * switchBlock)
	{
		llvm::Module * mod = func->getParent();
		llvm::LLVMContext & context = mod->getContext();

		llvm::SwitchInst * switchInst = llvm::cast<llvm::SwitchInst>(switchBlock->getTerminator());
		llvm::Value * switchValue = switchInst->getCondition();

		llvm::BasicBlock * defaultDest = switchInst->getDefaultDest();
		llvm::BasicBlock * exitBlock = defaultDest;

		//create IF-cascade (skip the default case)
		for (auto & caseHandle : switchInst->cases()) {
			uint i = caseHandle.getCaseIndex();
			llvm::Value * succVal = caseHandle.getCaseValue();
			llvm::BasicBlock * succBlock = caseHandle.getCaseSuccessor();

			{
				llvm::BasicBlock * caseBlock = llvm::BasicBlock::Create(context, "cascade" + str<uint>(i), func,  exitBlock);
				llvm::CmpInst * testInst = llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, switchValue, succVal, "caseCompare", caseBlock);
				llvm::BranchInst::Create(succBlock, exitBlock, testInst, caseBlock);

				// replace incoming from @switchBlock with incoming from cascade in default block
				if (exitBlock == defaultDest) {
					for (llvm::BasicBlock::iterator itPhi = exitBlock->begin(); llvm::isa<llvm::PHINode>(itPhi); ++itPhi)
					{
						llvm::PHINode * phi = llvm::cast<llvm::PHINode>(itPhi);
						for(uint i = 0; i < phi->getNumIncomingValues(); i++)
						{
							if (phi->getIncomingBlock(i) == switchBlock) {
								phi->setIncomingBlock(i, caseBlock);
							}
						}
					}
				}

				exitBlock = caseBlock;
			}
		}

		//replace Switch-Instruction
		switchInst->eraseFromParent();
		llvm::BranchInst::Create(exitBlock, switchBlock);
	}

	bool SimpleUnswitchPass::runOnFunction(llvm::Function * func)
	{
		bool foundSwitch = false;
		for (llvm::Function::iterator block = func->begin(); block != func->end(); ++block)
		{
			llvm::Instruction * termInst = block->getTerminator();
			if (llvm::isa<llvm::SwitchInst>(termInst)) {
				processSwitch(func, &*block);
				foundSwitch = true;
			}
		}
		return foundSwitch;
	}

	bool SimpleUnswitchPass::runOnModule(llvm::Module & M)
	{
		bool changed = false;

		for (llvm::Module::iterator func = M.begin(); func != M.end(); ++func)
		{
			changed |= runOnFunction(&*func);
		}

		return changed;
	}
}
