/*
 * PredicateRestruct.cpp
 *
 *  Created on: 27 Feb 2012
 *      Author: v1smoll
 */

#include <axtor/solvers/PredicateRestruct.h>

#include <axtor/util/llvmConstant.h>
#include <axtor/util/llvmDebug.h>
#include <llvm/Support/CFG.h>

namespace axtor {

PredicateRestruct::PredicateRestruct() {}
PredicateRestruct::~PredicateRestruct() {}

llvm::BasicBlock * PredicateRestruct::resolve(RegionVector & regions, llvm::BasicBlock * requiredExit, const ExtractorContext & context, AnalysisStruct & analysis)
{
	llvm::LLVMContext & LLVMContext = SharedContext::get();

	assert(false && "not yet implemented");

	//split all exits until the regions satisfy the single exit node property
	BlockSet regularExits = context.getRegularExits();
	BlockSetPair exitInfo = computeExitSet(regions, analysis.getDomTree(), context.getAnticipatedExits());
	BlockSet exitSet = exitInfo.first;
	BlockSet usedAnticipated = exitInfo.second;

	// create a new fused block
	llvm::BasicBlock * fusedBlock = llvm::BasicBlock::Create(LLVMContext, "fused", analysis.getFunction());
	llvm::PHINode * indexPHI = llvm::PHINode::Create(llvm::Type::getInt32Ty(LLVMContext), 6, "index", fusedBlock);

	// index exiting blocks
	BlockVector indexedExits(exitSet.size());
	uint index = 0;

	for (BlockSet::iterator itExitBlock = exitSet.begin(); itExitBlock != exitSet.end(); ++itExitBlock, ++index)
	{
		llvm::BasicBlock * exitBlock = *itExitBlock;
		indexedExits.push_back(exitBlock);

		// patch-up all predecessors
		llvm::pred_iterator itPredBlock, S, E;
		S = llvm::pred_begin(exitBlock);
		E = llvm::pred_end(exitBlock);

		ValueMap fixPHIMap;

		for (itPredBlock = S; itPredBlock != E; ++itPredBlock)
		{
			llvm::BasicBlock * predBlock = *itPredBlock;
			llvm::TerminatorInst * termInst = predBlock->getTerminator();
			int opIdx = getSuccessorIndex(termInst, exitBlock);

			assert(opIdx && "block unused by predecessor?!");

			llvm::Value * valueForBlock = indexPHI->getIncomingValueForBlock(predBlock);

			// insert an intermediate block
			if (valueForBlock) {
				llvm::BasicBlock * branchBlock = llvm::BasicBlock::Create(LLVMContext, "detachedBranch", analysis.getFunction(),fusedBlock);
				llvm::BranchInst::Create(fusedBlock, branchBlock);
				termInst->setOperand(opIdx, branchBlock);
				// Maintain that all PHI-nodes after the fuse block
				// refer to direct predecessors of the fused block
				fixPHIMap[exitBlock] = branchBlock;
				indexPHI->addIncoming(get_int(index), branchBlock);

			} else {
				termInst->setOperand(opIdx, fusedBlock);
				indexPHI->addIncoming(get_int(index), predBlock);
			}
		}

		// Move all PHIs to the fuseBlock
		for (llvm::BasicBlock::iterator itPHI = exitBlock->begin(); llvm::isa<llvm::PHINode>(itPHI); itPHI = exitBlock->begin())
		{
			itPHI->moveBefore(indexPHI);
		}

		LazyRemapBlock(exitBlock, fixPHIMap);
	}

	// Reattach the exitNodes to the fused Block (think switch)
	llvm::BasicBlock * altBlock = *indexedExits.begin();
	for (uint i = 1; i < indexedExits.size(); ++i)
	{
		llvm::BasicBlock * exitBlock = indexedExits[i];

		llvm::Constant * caseLabel = get_int(i);
		llvm::BasicBlock * caseBlock;

		// put the fusedBlock at the end of the cascade
		if (i < indexedExits.size() - 1) {
			caseBlock = llvm::BasicBlock::Create(LLVMContext, "fuseCascade" + str<uint>(i), analysis.getFunction(),  exitBlock);
		} else {
			caseBlock = fusedBlock;
		}

		llvm::CmpInst * testInst = llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, indexPHI, caseLabel, "caseCompare", caseBlock);
		llvm::BranchInst::Create(exitBlock, altBlock, testInst, caseBlock);

		// cascade
		altBlock = caseBlock;
	}

	return 0;
}

}


