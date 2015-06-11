/*
 * ReForPass.h
 *
 *  Created on: Jun 10, 2015
 *      Author: Simon Moll
 */

#ifndef INCLUDE_AXTOR_PASS_REFORPASS_H_
#define INCLUDE_AXTOR_PASS_REFORPASS_H_

// FOR (;;) {} restructuring pass
// Recovers canonical for loops from the CFG

// if (C(I))  do {<loopBody>; I=f(I);} while(C(I));
// ==> for (; C(I); I=f(I)); { loopBody }


#include <llvm/Pass.h>

namespace llvm {
	class LoopInfo;
	class ScalarEvolution;
	class Loop;
	class Function;
}

namespace axtor {

class ReForPass : public llvm::FunctionPass {
	llvm::LoopInfo * LI;
	llvm::ScalarEvolution * SE;
	llvm::Function * func;

	bool recoverForLoop(llvm::Loop * l);
	void visit(llvm::Loop * l);

public:
	static char ID;
	void getAnalysisUsage(llvm::AnalysisUsage & AU) const;
	bool runOnFunction(llvm::Function & F);
	ReForPass();
};

} /* namespace axtor */


#endif /* INCLUDE_AXTOR_PASS_REFORPASS_H_ */
