/*
 * ReForPass.cpp
 *
 *  Created on: Jun 10, 2015
 *      Author: Simon Moll
 */

#include <axtor/pass/ReForPass.h>

#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/CFG.h>

#include <axtor/util/llvmLoop.h>

#include <axtor/console/CompilerLog.h>

#include <llvm/IR/IRBuilder.h>
#include <axtor/util/llvmDebug.h>

using namespace llvm;

namespace axtor {

llvm::RegisterPass<ReForPass> __regReForPass("refor", "axtor - for loop restructure pass",
					false,
                    false);

char ReForPass::ID = 0;

ReForPass::ReForPass()
: FunctionPass(ID)
, LI(nullptr)
, SE(nullptr)
, func(nullptr)
{}


void
ReForPass::getAnalysisUsage(AnalysisUsage & AU) const {
	AU.addRequired<ScalarEvolutionWrapperPass>();
	AU.addRequired<LoopInfoWrapperPass>();
}

// moves backwards in the CFG along unique predecessor edges until it hits a conditional branch
// returns that block, or nullptr if it does not exist
static
BranchInst*
FindLastConditionalBranch(BasicBlock * block, BasicBlock * lastBlock, bool & onTrue) {
	BranchInst * branch = dyn_cast<BranchInst>(block->getTerminator());
	if (!branch) {
		return nullptr;
	}
	if (branch->isConditional()) {
		onTrue = branch->getSuccessor(0) == lastBlock;
		return branch;
	}

	BasicBlock * uniquePred = block->getUniquePredecessor();
	if (!uniquePred) {
		return nullptr;
	}

	return FindLastConditionalBranch(uniquePred, block, onTrue);
}

// Extracts a PHI node of the loop header that has a loop-carried predecessor that is immediately used in the exiting branch condition of the loop
// does not check for uniqueness
// returns the latchValue entering the PHI node
PHINode *
GetInductivePHI(Loop * l, Value *& latchValue) {
	auto * phi = l->getCanonicalInductionVariable();
	auto * header = l->getHeader();
	auto * latch = l->getLoopLatch();

	if (phi) {
		latchValue = phi->getIncomingValueForBlock(latch);
		return phi;
	}

	assert(header && latch);

	// BranchInst * exitBranch = cast<BranchInst>(l->getExitingBlock()->getTerminator());
	// User * cond = cast<User>(exitBranch->getCondition());

	for (Instruction & inst : *header) {
		phi = dyn_cast<PHINode>(&inst);
		if (!phi) return nullptr;

		latchValue = phi->getIncomingValueForBlock(latch);
		if (latchValue) return phi;
	}

	return nullptr;
}

// FIXME this is presumptious. Some aspects:
// - the condition may involve side effects. The old condition has to be removed explicitly (this could be done using the @seen list of Isomorph)
bool
ReForPass::recoverForLoop(Loop * l) {
	// loop condition is checked on the next iteration of the induction variable C(f(phi(I, ..)))
	Value * latchValue = nullptr;
	PHINode * loopPHI = GetInductivePHI(l, latchValue);
	assert(loopPHI);

	// FIXME PolloCL vector loop hack
	if (! loopPHI->getMetadata("pollocl.vectorize")) {
		return false;
	}

	BasicBlock * loopHeader = l->getHeader();
	BasicBlock * exitingBlock = l->getExitingBlock();
	BasicBlock * exitBlock = l->getExitBlock();
	BasicBlock * latchBlock = l->getLoopLatch();

	assert(loopHeader && exitingBlock && exitBlock && latchBlock);


	// latch must be exiting
	assert (latchBlock == exitingBlock);

	// loop predecessor
	BasicBlock * enteringBlock = l->getLoopPredecessor();
	bool preOnTrue;
	BranchInst * preBranch = FindLastConditionalBranch(enteringBlock, loopHeader, preOnTrue);
	assert(preBranch);

	// pre-condition must exit on loopExit
	// assert(preBranch->getSuccessor(preOnTrue ? 1 : 0) == exitBlock);

	// extract beginValue of the loop
	Value * beginValue = loopPHI->getIncomingValueForBlock(enteringBlock);

	// extract conditions
	BranchInst * exitingBranch = cast<BranchInst>(exitingBlock->getTerminator());
	bool exitOnFalse = exitingBranch->getSuccessor(0) == loopHeader;
	Value * exitCond = exitingBranch->getCondition();
	Value * preCond = preBranch->getCondition();

	// the preCond and exitCond are isomorph modulo the induction variable

	ValueToValueMapTy condRemap;

	CmpInst * condInst = cast<CmpInst>(exitCond);
	bool isoWithIncrement = false;
	{
		ValueToValueMapTy isoMap;
		isoMap[beginValue] = latchValue;
		errs() << "Map: " << *beginValue << " ~> " << *latchValue << "\n";
		isoWithIncrement = Isomorph(preCond, exitCond, isoMap);
	}

	CmpInst::Predicate cmpPred = condInst->getPredicate();
	bool isoWithoutIncrement = false;

	if (isoWithIncrement) {
		Log::warn(exitCond, "has exit condition with increment");
		condRemap[latchValue] = loopPHI; // fold back the outer loop guard into the loop's exit branch
	} else {
		Log::warn(preCond, "could now show isomorphism! Forcing for conversion.. (hack for Pollocl)");
		ValueToValueMapTy isoMap;
		isoMap[beginValue] = loopPHI;
		errs() << "Map: " << *beginValue << " ~> " << *loopPHI << "\n";
		isoWithoutIncrement = Isomorph(preCond, exitCond, isoMap);

		if (isoWithoutIncrement) {
			Log::warn(exitCond, "has exit condition without increment");
			// modify the predicate to include equivalence
			assert(! CmpInst::isTrueWhenEqual(cmpPred) && "otw this hack will not work");
			cmpPred = (CmpInst::Predicate) (cmpPred | CmpInst::FCMP_OEQ);
		}
	}

	condInst->setPredicate(cmpPred); // EQ bit

	assert(isoWithIncrement || isoWithoutIncrement);

// Rebuild for-loop

	// make preBranch unconditional
	BasicBlock * loopPathEntry = preBranch->getSuccessor(preOnTrue ? 0 : 1);
	BranchInst::Create(loopPathEntry, preBranch);
	preBranch->eraseFromParent();

	// Split loop header after PHI
	BasicBlock * loopBody = loopHeader->splitBasicBlock(loopHeader->getFirstNonPHIOrDbgOrLifetime(), "body");

	// Move exiting branch to header
	BranchInst::Create(loopHeader, exitingBranch); // unconditional back edge

	exitingBranch->removeFromParent();
	exitingBranch->setSuccessor(exitOnFalse ? 0 : 1, loopBody); // jump to body to continue

	Instruction * bodyBranch = loopHeader->getTerminator();
	exitingBranch->insertBefore(bodyBranch);
	bodyBranch->eraseFromParent();

	// Remap f(I) to I in exit condition
	// FIXME: this is too simplistic (loop-carried stuff)
	RemapInstruction(condInst, condRemap);
	// RemapInstruction(condInst, condRemap, RF_IgnoreMissingEntries);
	condInst->removeFromParent();
	condInst->insertBefore(exitingBranch);

	// assure that the increment goes before the condition (if it is used in the condition)
	if (isoWithIncrement) {
		Instruction * latchIncrement = cast<Instruction>(latchValue);
		latchIncrement->removeFromParent();
		latchIncrement->insertBefore(condInst);
	}

// Re-pair PHI nodes in exit block
	// FIXME this implicitly assumes that is is safe to make PHI-Nodes insensitive to @preCond
	// TODO use isomorphism to prove this in most cases
	for (Instruction & inst : *exitBlock) {
		PHINode * phi = dyn_cast<PHINode>(&inst);
		if (!phi) break;

		phi->removeIncomingValue(preBranch->getParent(), true);
	}
	return true;
}

void
ReForPass::visit(Loop * l) {
	for (Loop * subLoop : *l) visit(subLoop);

	ForLoopInfo forInfo;
	if (inferForLoop(l, forInfo)) {
		return;
	}

	if (recoverForLoop(l)) {
		llvm::errs() << *func;
	}

	/*
	for (Instruction & inst : *l->getHeader()) {
		PHINode * phi = dyn_cast<PHINode>(inst);
		if (phi->getMetadata("pollocl.vectorize")) {
			return;
		}
	}*/
}

bool
ReForPass::runOnFunction(Function & F) {
	func = &F;
	LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
	SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

	for (Loop * l : *LI) {
		visit(l);
	}

	llvm::errs() << F;
	// abort();

	return true;
}


} /* namespace axtor */
