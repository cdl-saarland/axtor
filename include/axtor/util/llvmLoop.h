/*
 * llvmLoop.h
 *
 *  Created on: Aug 6, 2010
 *      Author: Simon Moll
 */

#ifndef LLVMLOOP_HPP_
#define LLVMLOOP_HPP_

#include <axtor/config.h>

#include <llvm/Analysis/LoopInfo.h>
#include <axtor/CommonTypes.h>

#include <llvm/IR/Instructions.h>
#include <set>


namespace llvm {
	class Loop;
	class Value;
}


typedef std::set<llvm::Value*> ValueSet;

namespace axtor {

	/*
	 * identify the immediate sub loop of @parent that contains @block.
	 *
	 * returns NULL if no such loop exists
	 */
	llvm::Loop * getNestedLoop(llvm::LoopInfo & loopInfo, llvm::Loop * parent, llvm::BasicBlock * block);


	struct ForLoopInfo {
		llvm::PHINode * phi;
		llvm::BasicBlock * headerBlock;
		llvm::BasicBlock * bodyBlock;
		llvm::Value * beginValue;
		llvm::CmpInst * exitCond;
		llvm::Value * ivIncrement;
		bool exitOnFalse;
		bool ivParallelLoop;

		ForLoopInfo()
		: phi(nullptr)
		, headerBlock(nullptr)
		, bodyBlock(nullptr)
		, beginValue(nullptr)
		, exitCond(nullptr)
		, ivIncrement(nullptr)
		, exitOnFalse(false)
		, ivParallelLoop(false)
		{}
	};

	/*
	 * Checks whether this is a for loop with an induction variable
	 * If so, it extracts various details about the loop structure/ exit condition, etc
	 */
	bool inferForLoop(llvm::Loop * l, ForLoopInfo & info);


	// expression isomorphism
	bool Isomorph(llvm::Value * A, llvm::Value * B, llvm::ValueToValueMapTy & mapping);
	bool Isomorph(llvm::Value * A, llvm::Value * B, llvm::ValueToValueMapTy & mapping, ValueSet&);

}


#endif /* LLVMLOOP_HPP_ */
