/*
 * NodeSplittingRestruct.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef NODESPLITTINGRESTRUCT_HPP_
#define NODESPLITTINGRESTRUCT_HPP_

#include "RestructuringProcedure.h"

#include <axtor/config.h>
#include <axtor/util/llvmShortCuts.h>
#include <axtor/util/llvmDuplication.h>

namespace axtor {

	/*
	 * acyclic restructuring procedure that uses a node splitting based algorithm
	 */
	class NodeSplittingRestruct : public RestructuringProcedure
	{

		static NodeSplittingRestruct instance;

		int findUniqueRegion(llvm::DominatorTree & domTree, RegionVector & regions, llvm::BasicBlock * block);

		BlockVector getAbstractSuccessors(llvm::BasicBlock * block, llvm::Loop * parentLoop, llvm::LoopInfo & loopInfo);

		/*
		 * split & rinse the top element
		 */
		llvm::BasicBlock * splitAndRinseOnStack(BlockVector & stack, const ExtractorContext & context, AnalysisStruct & analysis, llvm::BasicBlock * mandatoryExit);

		/*
		 * tries to generate a minimal number of splits for this block (one clone for each region)
		 */
		llvm::BasicBlock * splitAndRinseOnStack_perRegion(RegionVector & regions, BlockVector & stack, const ExtractorContext & context, AnalysisStruct & analysis, llvm::BasicBlock * mandatoryExit);

		/*
		 * sorts a given vector using the mutual node reachability as a relation
		 */
		void sortByReachability(BlockVector & vector, BlockSet regularExits);
		void sortByReachability(BlockVector & vector, int start, int end, BlockSet regularExits);

		/*
		 * returns a basic block that is unreachable from any other in @blocks
		 *
		 * this will always return a block if the reachability graph is acyclic
		 */
		llvm::BasicBlock * getUnreachableBlock(BlockSet blocks, BlockSet anticipated);

	public:
		NodeSplittingRestruct();
		~NodeSplittingRestruct();

		virtual bool resolve(RegionVector & regions, llvm::BasicBlock * requiredExit, const ExtractorContext & context, AnalysisStruct & analysis, llvm::BasicBlock* & oExitBlock);

		static NodeSplittingRestruct * getInstance();
	};
}


#endif /* NODESPLITTINGRESTRUCT_HPP_ */
