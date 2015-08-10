/*
 * llvmLoop.cpp
 *
 *  Created on: Aug 6, 2010
 *      Author: Simon Moll
 */

#include <axtor/util/llvmLoop.h>
#include <axtor/config.h>
#include <axtor/console/CompilerLog.h>

using namespace llvm;

namespace axtor {

	llvm::Loop * getNestedLoop(llvm::LoopInfo & loopInfo, llvm::Loop * parent, llvm::BasicBlock * block)
	{
		if (parent) { //get a directly nested subloop that contains @block if any
			const LoopVector & subLoops = parent->getSubLoops();

			for(LoopVector::const_iterator itLoop = subLoops.begin(); itLoop != subLoops.end(); ++itLoop)
			{
				llvm::Loop * subLoop = *itLoop;
				if (subLoop->contains(block))
				{
					return subLoop;
				}
			}

			return 0;

		} else { //return the outermost loop for this block
			llvm::Loop * innerMostLoop = loopInfo.getLoopFor(block);
			llvm::Loop * loop = innerMostLoop;

			while(loop != 0 && loop->getParentLoop() != 0)
			{
				loop = loop->getParentLoop();
			}

			return loop;
		}
	}

	bool
	Isomorph(Value * A, Value * B, ValueToValueMapTy & mapping) {
		ValueSet seen;
		return Isomorph(A, B, mapping, seen);
	}

	// checks whether two expression are structurally isomorphic modulo @mapping
	// mapping implicitely contains all pairs (v,v)
	bool
	Isomorph(Value * A, Value * B, ValueToValueMapTy & mapping, ValueSet& seen) {
		assert(A && B);

		if (! seen.insert(A).second)
			return true;

		llvm::errs() << "[ISO] " << *A << " ~~" << *B << "\n";

		if (A == B) return true;
		if (mapping[A] == B) return true;

		Instruction * aInst = dyn_cast<Instruction>(A);
		Instruction * bInst = dyn_cast<Instruction>(B);
		if (aInst && bInst) {
			if (aInst->getOpcode() != bInst->getOpcode()) {
				return false;
			}
		} else if (aInst || bInst) return false;

		User * aUser = cast<User>(A);
		User * bUser = cast<User>(B);

		assert(aUser->getNumOperands() == bUser->getNumOperands());
		for (unsigned i = 0; i < aUser->getNumOperands(); ++i) {
			if (! Isomorph(aUser->getOperand(i), bUser->getOperand(i), mapping, seen))
				return false;
		}

		// at least, we could not refute equivalence..
		return true;
	}

	static PHINode *
	FindInductionVariable(Loop * l) {
		PHINode * phi = l->getCanonicalInductionVariable();
		if (phi) return phi;

		BasicBlock * header = l->getHeader();
		PHINode * lastPHI = nullptr;
		for (Instruction & inst : *header) {
			PHINode * phi = dyn_cast<PHINode>(&inst);
			if (!phi) break;
			if (phi && lastPHI) return nullptr; // multiple PHIs in loop
			lastPHI = phi;
		}

		return lastPHI;
	}


	bool inferForLoop(Loop * l, ForLoopInfo & I) {
		// there is an unique induction variable
		I.phi = FindInductionVariable(l);
		if (!I.phi) {
			IF_DEBUG l->getHeader()->dump();
			IF_DEBUG Log::warn("Could not find canonical induction variable");
			return false;
		}

		I.ivParallelLoop =
					(I.phi->getMetadata("pollocl.vectorize") != nullptr);

		// there is an unique pre-header
		BasicBlock * preHeader = l->getLoopPreheader();
		if (!preHeader) {
			IF_DEBUG Log::warn("Could not find pre-header");
			return false;
		}

		// header control the loop exit
		BasicBlock * header = l->getHeader();
		BasicBlock * latch = l->getLoopLatch();

		I.beginValue = I.phi->getIncomingValueForBlock(preHeader);
		I.ivIncrement = I.phi->getIncomingValueForBlock(latch);
		I.headerBlock = l->getHeader();
		bool headerControlsLoop = l->getExitingBlock() == I.headerBlock;
		if (!headerControlsLoop) {
			IF_DEBUG Log::warn("Exit branch is not in the loop header");
			return false;
		}

		// exit branch is conditional
		BranchInst * exitBranch = dyn_cast<BranchInst>(header->getTerminator());
		if (!exitBranch || !exitBranch->isConditional()) {
			IF_DEBUG Log::warn("Exit branch is not-conditional or not unique");
			return false;
		}

		// exit condition is a CmpInst
		Value * condition = exitBranch->getCondition();
		I.exitCond = dyn_cast<CmpInst>(condition);
		I.exitOnFalse = l->contains(exitBranch->getSuccessor(0));
		if (!I.exitCond) {
			IF_DEBUG Log::warn("No CmpInst-based exit condition");
			return false;
		}

#if 0
		// the compare operates on the induction variable and a bound value
		Value * cmpLeft = I.exitCond->getOperand(0);
		Value * cmpRight = I.exitCond->getOperand(1);
		// Value * endValue = nullptr;
		CmpInst::Predicate exitPred;
		if (cmpRight == I.phi) {
			// endValue = cmpLeft;
			exitPred = I.exitCond->getSwappedPredicate();

		} else 	if (cmpLeft == I.phi) {
			// endValue = cmpRight;
			exitPred = I.exitCond->getPredicate();
		} else {
			IF_DEBUG Log::warn(I.exitCond, "ivPHI is not immediate operand of exit CmpInst");
			return false;
		}

		// modify the predicate to match exit-on-false
		if (! I.exitOnFalse)
			exitPred = CmpInst::getInversePredicate(exitPred);
#endif

		if (I.exitOnFalse) {
			I.bodyBlock = exitBranch->getSuccessor(0);
		} else {
			I.bodyBlock = exitBranch->getSuccessor(1);
		}

		if (I.bodyBlock == l->getHeader()) {
			IF_DEBUG Log::warn("Single block loops are never for loops");
			return false;
		}

		// Check if this loop is annotated as induction-variable-parallel
		IF_DEBUG I.phi->dump();

		return true;
	}

} /*  namespace llvm */
