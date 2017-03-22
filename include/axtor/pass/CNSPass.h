/*
 * Regularizer.h
 *
 *  Created on: 29.04.2010
 *      Author: Simon Moll
 */

#ifndef REGULARIZER_HPP_
#define REGULARIZER_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>

#include <axtor/pass/TargetProvider.h>

#include <axtor/pass/OpaqueTypeRenamer.h>
#include <axtor/cns/BlockGraph.h>
#include <axtor/cns/CNS.h>
#include <axtor/cns/CNSScoring.h>
#include <axtor/cns/SplitTree.h>
#include <axtor/util/ResourceGuard.h>
#include <axtor/util/llvmDuplication.h>



/*
 * The Regularizer makes irreducible control flow reducible by applying controlled node splitting
 */
namespace axtor {

class CNSPass : public llvm::ModulePass
{
	cns::SplitTree * generateSplitSequence(cns::SplitTree * root, BlockGraph::SubgraphMask & mask, BlockGraph & graph);

	void applySplitSequence(BlockGraph & graph, std::vector<uint> nodes) const;

	bool runOnFunction(llvm::Function & func);
public:
	static char ID;

	CNSPass() :
		llvm::ModulePass(ID)
	{}

	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

	bool runOnModule(llvm::Module & M);

	virtual llvm::StringRef getPassName() const;
};

}


#endif /* REGULARIZER_HPP_ */
