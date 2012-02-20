/*
 * llvmDuplication.cpp
 *
 *  Created on: Jun 21, 2010
 *      Author: moll
 */

#include <axtor/util/llvmDuplication.h>
#include <llvm/PassManagers.h>
//#include <llvm/Transforms/Utils/ValueMapper.h>
#include <axtor/util/llvmDebug.h>

#include <axtor/util/llvmDomination.h>

namespace axtor {

BlockSet splitNode(BlockCopyTracker & tracker, llvm::BasicBlock * srcBlock)
{
	return splitNode(srcBlock);
}

BlockSet splitNode(llvm::BasicBlock * srcBlock, llvm::DominatorTree * domTree)
{
	assert(srcBlock && "was NULL");

	BlockVector preds = getAllPredecessors(srcBlock);
	BlockSet clones;

	if (preds.size() <= 1)
		return clones;

	if (domTree) {
		llvm::BasicBlock * first = *preds.begin();
		domTree->changeImmediateDominator(srcBlock, first);
	}

	for(BlockVector::iterator itPred = (preds.begin() + 1); itPred != preds.end(); ++itPred)
	{
		llvm::BasicBlock * predBlock = *itPred;
		llvm::BasicBlock * clonedBlock = cloneBlockForBranch(srcBlock, predBlock, domTree);
		clones.insert(clonedBlock);
	}

	return clones;
}

BlockVector splitNodeExt(llvm::BasicBlock * srcBlock, BlockSetVector predecessorSet, llvm::DominatorTree * domTree)
{
	assert(srcBlock && "was NULL");

	BlockVector clones;

	if (predecessorSet.size() <= 1)
		return clones;

	if (domTree) {
		BlockSet firstSet = *predecessorSet.begin();
		if (firstSet.size() == 1)
			domTree->changeImmediateDominator(srcBlock, *firstSet.begin());
	}

	for(BlockSetVector::iterator itSet = (predecessorSet.begin() + 1); itSet != predecessorSet.end(); ++itSet)
	{
		if (! (*itSet).empty()) {
			llvm::BasicBlock * clonedBlock = cloneBlockForBranchSet(srcBlock, *itSet, domTree);
			clones.push_back(clonedBlock);
		} else {
			clones.push_back(0);
		}
	}

	return clones;
}


LoopSet splitLoop(BlockCopyTracker & tracker, llvm::LoopInfo & loopInfo, llvm::Loop * loop, llvm::Pass * pass)
{
	return splitLoop(loopInfo, loop, pass);
}

LoopSet splitLoop(llvm::LoopInfo & loopInfo, llvm::Loop * loop, llvm::Pass * pass, llvm::DominatorTree * domTree)
{
	assert(loop && "was NULL");
	assert(pass && "sorry, somehow llvm::CloneLoop needs as pass object");

	llvm::BasicBlock * header = loop->getHeader();


	BlockVector preds = getAllPredecessors(header);

	LoopSet clones;

	if (preds.size() <= 1)
		return clones;

	if (domTree) {
		llvm::BasicBlock * first= *preds.begin();
		llvm::BasicBlock * header = loop->getHeader();
		domTree->changeImmediateDominator(header, first);
	}

	loop->getHeader();

	for(BlockVector::iterator itPred = (preds.begin() + 1); itPred != preds.end(); ++itPred)
	{
		//llvm::BasicBlock * predBlock = *itPred;

		llvm::LPPassManager lpm;
    //FIXME
		//llvm::Loop * clonedLoop = cloneLoopForBranch(lpm, pass, loopInfo, loop, predBlock, domTree);
		//clones.insert(clonedLoop);
	}

	return clones;
}

llvm::BasicBlock * cloneBlockAndMapInstructions(llvm::BasicBlock * block, ValueMap & cloneMap)
{
	llvm::BasicBlock * clonedBlock = llvm::CloneBasicBlock(block, cloneMap, "", block->getParent());

	for(llvm::BasicBlock::iterator inst = clonedBlock->begin(); inst != clonedBlock->end(); ++inst)
	{
	//	std::cerr << "remapping inst=" << inst->getName().str() << std::endl;
		LazyRemapInstruction(inst, cloneMap);
	}

	return clonedBlock;
}


//FIXME
/*llvm::Loop * cloneLoopForBranch(BlockCopyTracker & tracker, llvm::LPPassManager & lpm, llvm::Pass * pass, llvm::LoopInfo & loopInfo, llvm::Loop * loop, llvm::BasicBlock * branchBlock)
{
	return cloneLoopForBranch(lpm, pass, loopInfo, loop, branchBlock);
}

llvm::Loop * cloneLoopForBranch(llvm::LPPassManager & lpm, llvm::Pass * pass, llvm::LoopInfo & loopInfo, llvm::Loop * loop, llvm::BasicBlock * branchBlock, llvm::DominatorTree *domTree)
{
	ValueMap cloneMap;
	llvm::Loop *clonedLoop = llvm::CloneLoop(loop, &lpm, &loopInfo, 
                                           cloneMap, pass);
	if (domTree)
		domTree->addNewBlock(clonedLoop->getHeader(), branchBlock);

	patchClonedBlocksForBranch(cloneMap, loop->getBlocks(), clonedLoop->getBlocks(), branchBlock);

	return clonedLoop;
}*/

//### fix up incoming values ###

void patchClonedBlocksForBranch(ValueMap & cloneMap, const BlockVector & originalBlocks, const BlockVector & clonedBlocks, llvm::BasicBlock * branchBlock)
{
	BlockSet branchSet; branchSet.insert(branchBlock);
	patchClonedBlocksForBranches(cloneMap, originalBlocks, clonedBlocks, branchSet);
}

void patchClonedBlocksForBranches(ValueMap & cloneMap, const BlockVector & originalBlocks, const BlockVector & clonedBlocks, BlockSet branchBlocks)
{
	for(BlockVector::const_iterator itBlock = originalBlocks.begin(); itBlock != originalBlocks.end(); ++itBlock)
	{
		llvm::BasicBlock * srcBlock = *itBlock;
		llvm::BasicBlock * clonedBlock = llvm::cast<llvm::BasicBlock>(cloneMap[srcBlock]);

		//tracker.identifyBlocks(srcBlock, clonedBlock);

		for(llvm::BasicBlock::iterator it = srcBlock->begin();
				it != srcBlock->end() && llvm::isa<llvm::PHINode>(it);
				)
		{
			llvm::PHINode * srcPHI = llvm::cast<llvm::PHINode>(it++);

			llvm::PHINode * clonedPHI = llvm::cast<llvm::PHINode>(cloneMap[srcPHI]);

			if (branchBlocks.size() == 1) {
				llvm::BasicBlock * branchBlock = *branchBlocks.begin();

				int blockIdx = clonedPHI->getBasicBlockIndex(branchBlock);

				if (blockIdx > -1) {
	#ifdef DEBUG
					std::cerr << "spezialising PHI for branch value.phi=" << clonedPHI->getName().str() << std::endl;
	#endif
					llvm::Value * branchValue = clonedPHI->getIncomingValue(blockIdx);
					//specialize PHI (in cloned block)
					clonedPHI->replaceAllUsesWith(branchValue);
	#ifdef DEBUG
					std::cerr << "replacing phi" << clonedPHI->getName().str() << " with " << branchValue->getName().str() << std::endl;
	#endif
					cloneMap[srcPHI] = branchValue;
					clonedPHI->eraseFromParent();

					//remove incoming edge from branchBlock (in source block)
					srcPHI->removeIncomingValue(branchBlock, true);
				}
			} else {
#ifdef DEBUG
				std::cerr << "## PHI before\n";
				clonedPHI->dump();
#endif
				for(uint i = 0; i < clonedPHI->getNumIncomingValues(); ++i) {
					llvm::BasicBlock * incBlock = clonedPHI->getIncomingBlock(i);
#ifdef DEBUG
					std::cerr << "incoming : " << incBlock->getNameStr() << "\n";
#endif
					if (!contains(clonedBlocks, incBlock) && !set_contains(branchBlocks, incBlock) ) {
#ifdef DEBUG
						std::cerr << "not from current set! remove";
#endif
						clonedPHI->removeIncomingValue(incBlock, false);
					}
				}
#ifdef DEBUG
				std::cerr << "## PHI after\n";
#endif
				clonedPHI->dump();
			}
		}

		//### fix up successors and effects on own instructions ###
		/*
		 * 1.) redirect branches from @clonedBlock and @branchBlock to @clonedBlock
		 * 2.) add incoming value from @clonedBlock to PHINodes in srcBlocks successors
		 */
		for(llvm::Value::use_iterator itUse = srcBlock->use_begin(); itUse != srcBlock->use_end();)
		{
			llvm::User * user = *itUse++;
#ifdef DEBUG
			std::cerr << "found block user:";
			user->dump();
#endif

			llvm::Instruction * inst = llvm::cast<llvm::Instruction>(user);
			if (contains<llvm::BasicBlock*>(originalBlocks, inst->getParent())) {
				continue;
				//branchInst = llvm::cast<llvm::BranchInst>( cloneMap[branchInst] );
			}

			if (llvm::isa<llvm::BranchInst>(inst))	{
				//### fix branches ###
				llvm::BranchInst * branchInst = llvm::cast<llvm::BranchInst>(inst);

				branchInst->getParent()->dump();

				//could be a branch from the branch block
				if (contains<llvm::BasicBlock*>(clonedBlocks, branchInst->getParent()) || set_contains(branchBlocks, branchInst->getParent()))
				{
					//RemapInstruction(branchInst, branchFixMap);
					LazyRemapInstruction(branchInst, cloneMap);
				}

			} else if (llvm::isa<llvm::PHINode>(inst)) {
				//### fix receiving PHIs ###
				llvm::PHINode * phi = llvm::cast<llvm::PHINode>(user);

				llvm::Value * inVal = phi->getIncomingValueForBlock(srcBlock);

				if (phi->getBasicBlockIndex(clonedBlock) == -1)
				{
#ifdef DEBUG
				std::cerr << "### fixed PHI for new incoming edge" << std::endl;
				inVal->dump();
#endif
					phi->addIncoming(cloneMap[inVal], clonedBlock);
				}
			}
		}
	}
}

llvm::BasicBlock * cloneBlockForBranch(BlockCopyTracker & tracker, llvm::BasicBlock * srcBlock, llvm::BasicBlock * branchBlock)
{
	return cloneBlockForBranch(srcBlock, branchBlock);
}
/*
 * creates a copy of @srcBlock that replaces the original srcBlock on the edge coming from branchBlock
 */
llvm::BasicBlock * cloneBlockForBranch(llvm::BasicBlock * srcBlock, llvm::BasicBlock * branchBlock, llvm::DominatorTree * domTree)
{
	BlockSet branchSet; branchSet.insert(branchBlock);
	return cloneBlockForBranchSet(srcBlock, branchSet, domTree);
}


llvm::BasicBlock * cloneBlockForBranchSet(llvm::BasicBlock * srcBlock, BlockSet branchSet, llvm::DominatorTree * domTree)
{
#ifdef DEBUG
	std::cerr << " cloning block : " << srcBlock->getName().str() << " for blockset : " << toString(branchSet) << "\n";
#endif

	//sanity check
	assert(! srcBlock->getUniquePredecessor() && "block already has only a single predecessor");

	ValueMap cloneMap;

	llvm::BasicBlock * clonedBlock = cloneBlockAndMapInstructions(srcBlock, cloneMap);
	cloneMap[srcBlock] = clonedBlock;
	ValueMap branchFixMap;
	//branchFixMap[srcBlock] = clonedBlock;

	//reattach branches
	BlockVector originalBlocks(1, srcBlock);
	BlockVector clonedBlocks(1, clonedBlock);

	patchClonedBlocksForBranches(cloneMap, originalBlocks, clonedBlocks, branchSet);

	// fix up the dominance tree
	if (domTree) {
		if (branchSet.size() == 1) {
			domTree->addNewBlock(clonedBlock, *(branchSet.begin()));
		} else {
			domTree->addNewBlock(clonedBlock, findImmediateDominator(*domTree, branchSet)->getBlock());
		}
	}


	return clonedBlock;
}

}
