/*
 * AnalysisStruct.h
 *
 *  Created on: 20.02.2011
 *      Author: Simon Moll
 */

#ifndef ANALYSISSTRUCT_HPP_
#define ANALYSISSTRUCT_HPP_


#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/PostDominators.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Analysis/ScalarEvolution.h>

#include <axtor/pass/AnalysisProvider.h>

namespace axtor{

	class AnalysisStruct
	{
		AnalysisProvider * provider;

		llvm::Function * func;
		llvm::LoopInfo * loopInfo;
		llvm::DominatorTree * domTree;
		llvm::PostDominatorTree * postDomTree;
		llvm::ScalarEvolution * SE;
	public:
		inline llvm::Function * getFunction() { return func; }
		inline llvm::LoopInfo & getLoopInfo() { return *loopInfo; }
		inline llvm::Loop * getLoopFor(const llvm::BasicBlock * block) { return loopInfo->getLoopFor(block); }

		inline const llvm::ScalarEvolution & getSE() { return *SE; }
		inline const llvm::SCEV * getSCEV(llvm::Value * val) const { return SE->getSCEV(val); }
		inline llvm::DominatorTree & getDomTree() { return *domTree; }
		inline bool dominates(const llvm::BasicBlock * a, const llvm::BasicBlock * b) { return domTree->dominates(a, b); }

		inline llvm::PostDominatorTree & getPostDomTree() { return *postDomTree; }
		inline bool postDominates(const llvm::BasicBlock * a, const llvm::BasicBlock * b) { return postDomTree->dominates(a, b); }

		AnalysisStruct()
		: provider(nullptr)
		, func(nullptr)
		, loopInfo(nullptr)
		, domTree(nullptr)
		, postDomTree(nullptr)
		, SE(nullptr)
		{}

		AnalysisStruct(AnalysisProvider & _provider, llvm::Function & _func, llvm::LoopInfo & _loopInfo, llvm::DominatorTree & _domTree, llvm::PostDominatorTree & _postDomTree, llvm::ScalarEvolution & _SE) :
			provider(&_provider), func(&_func), loopInfo(&_loopInfo), domTree(&_domTree), postDomTree(&_postDomTree),
			SE(&_SE)
		{}

		void rebuild();
	};
}

#endif /* ANALYSISSTRUCT_HPP_ */
