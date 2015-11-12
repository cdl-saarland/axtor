/*
 * RestructuringPass.cpp
 *
 *  Created on: 20.02.2011
 *      Author: gnarf
 */


#include <axtor/pass/RestructuringPass.h>

#include <axtor/solvers/RestructuringProcedure.h>
#include <axtor/console/CompilerLog.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <axtor/util/llvmDebug.h>

#include <llvm/Analysis/ScalarEvolution.h>

#include <axtor/config.h>

using namespace llvm;

namespace axtor {

	llvm::RegisterPass<RestructuringPass> __regRestructPass(
			"restruct", "axtor - the new modular restructuring ast-extraction pass",
			false, /* mutates the CFG */
			false); /* transformation */

	char RestructuringPass::ID = 0;

	ast::ControlNode * RestructuringPass::processRegion(bool enteredLoop, const ExtractorRegion & region, AnalysisStruct & analysis, BlockSet & visited)
	{
		ast::NodeVector nodes;
		const ExtractorContext & context = region.context;

		IF_DEBUG {
			llvm::errs() << "processRegion : " << region.getHeader()->getName().str() << " until " << (context.exitBlock ? context.exitBlock->getName().str()  : "NULL") << "\n";
			llvm::errs() << "{\n";
		}
		//llvm::BasicBlock * exitBlock = region.context.exitBlock;


		llvm::BasicBlock * exitBlock = context.exitBlock;
		llvm::BasicBlock * block = region.getHeader();
		ast::ControlNode * childNode;

		while (block != NULL)
		{
			llvm::BasicBlock * next = processBasicBlock(enteredLoop, block, context, analysis, visited, childNode);
			enteredLoop = false; //following nodes are listed behind in a sequence
			nodes.push_back(childNode);
			block = next;
/*#ifdef DEBUG
			llvm::errs() << "returned " << (block ? block->getName().str() : "NULL") << "\n";
#endif*/
			if (block == exitBlock)
				break;
		}

		IF_DEBUG llvm::errs() << "}\n";

		if (nodes.size() == 1) {
			return childNode;
		} else {
			return ast::ASTFactory::createList(nodes);
		}
	}

	void RestructuringPass::rebuildAnalysisStruct(llvm::Function & func, AnalysisStruct & analysis)
	{
		llvm::LoopInfo & funcLoopInfo = getAnalysis<llvm::LoopInfoWrapperPass>(func).getLoopInfo();
		llvm::DominatorTree & funcDomTree = getAnalysis<llvm::DominatorTreeWrapperPass>(func).getDomTree();
		llvm::PostDominatorTree & funcPostDomTree = getAnalysis<llvm::PostDominatorTree>(func);
		llvm::ScalarEvolution & SE = getAnalysis<llvm::ScalarEvolutionWrapperPass>(func).getSE();
		analysis = AnalysisStruct(*this, func, funcLoopInfo, funcDomTree, funcPostDomTree, SE);
	}


	llvm::BasicBlock * RestructuringPass::processBasicBlock(bool enteredLoop, llvm::BasicBlock * bb, const ExtractorContext & context, AnalysisStruct & analysis, BlockSet & visited, ast::ControlNode *& oNode)
	{

		assert(bb);
		assert(!llvm::isa<llvm::SwitchInst>(bb->getTerminator()) && "does not support switches!");
		IF_DEBUG llvm::errs() << "\tprocessBlock : " << bb->getName() << "\n";

		/*
		 * check special branch cases (zero payload nodes)
		 */

		if (!enteredLoop) {
			if (bb == context.continueBlock) {
				IF_DEBUG llvm::errs() << "\t\t(continue)" << "\n";
				oNode = new ast::ContinueNode(NULL); //preserve AST semantics
				return NULL;
			} else if (bb == context.breakBlock) {
				IF_DEBUG llvm::errs() << "\t\t(break)" << "\n";
				oNode = new ast::BreakNode(NULL); //preserve AST-semantics
				return NULL;
			}
		}


		/*
		 * check special blocks (with payload)
		 */
		llvm::TerminatorInst * termInst = bb->getTerminator();
		if (termInst->getNumSuccessors() == 0) {
			if (llvm::isa<llvm::ReturnInst>(termInst)) {
				IF_DEBUG llvm::errs() << "\t\t(return)" << "\n";
				oNode = new ast::ReturnNode(bb);
				return NULL;

			} else if (llvm::isa<llvm::UnreachableInst>(termInst)) {
				IF_DEBUG llvm::errs() << "\t\t(unreachable)" << "\n";
				oNode = new ast::UnreachableNode(bb);
				return NULL;
			}
		}



		/*
		 * check loops (this will abort the program if it encounters unstructured loops)
		 */
		PrimitiveParser::BuilderSession * loopBuilder = parsers.loopParser->tryParse(bb, context, analysis);
		if (loopBuilder)
		{
			IF_DEBUG llvm::errs() << "\t\t(loop)" << "\n";
			ast::NodeMap children;
			const ExtractorRegion & region = loopBuilder->getRegion(0);
			assert(region.verify(analysis.getDomTree()) && "loop body uses non-anticipated exits");

			IF_DEBUG {
				llvm::errs() << "\tloop body";
				region.dump("\t\t");
			}

			children[region.getHeader()] = processRegion(true, region, analysis, visited);
			oNode = loopBuilder->build(children, NULL);

			return loopBuilder->getRequiredExit();
		}


		/*
		 * check acyclic control primitives
		 */

		/*
		 * 1-way primitives
		 */
		if (termInst->getNumSuccessors() == 1)
		{
			llvm::BasicBlock * next = termInst->getSuccessor(0);

			IF_DEBUG llvm::errs() << "\t\t(sequence)" << "\n";
			oNode = new ast::BlockNode(bb);
			return next;

		}


		/*
		 * 2-way primitive
		 */
		PrimitiveParser::BuilderSession * ifBuilder = parsers.ifParser->tryParse(bb, context, analysis);
		if (ifBuilder)
		{
			IF_DEBUG llvm::errs() << "\t\tis acyclic primitive (builder = " << ifBuilder->getName() << ")\n";
			ast::NodeMap children;
			RestructuringProcedure * solver = ifBuilder->getSolver();
			assert(solver && "solvers mandatory for acyclic primitives");

			// resolve control-flow issues (single node exit property)
			llvm::BasicBlock * exitBlock = 0;
			while (solver->resolve(ifBuilder->getRegions(), ifBuilder->getRequiredExit(), context, analysis, exitBlock)) {
				ifBuilder = parsers.ifParser->tryParse(bb, context, analysis);
			}

			RegionVector & regions = ifBuilder->getRegions();

			// recurse over all child regions
			for (RegionVector::iterator itRegion = regions.begin(); itRegion != regions.end(); ++itRegion)
			{
				ExtractorRegion & region = *itRegion;
#ifdef DEBUG
				region.dump("\t");
#endif

				EXPENSIVE_TEST if (! region.verify(analysis.getDomTree())) {
					analysis.getLoopInfo().print(llvm::errs());
					analysis.getDomTree().print(llvm::errs());
#ifdef DEBUG_VIEW_CFGS
					llvm::errs() << "VIEWCFG: with invalid regions at header " << (region.getHeader() ? region.getHeader()->getName() : "0") << "\n";
					bb->getParent()->viewCFGOnly();
#endif
					Log::fail(bb->getParent(), "region invalid after solving");
					assert(false && "region invalid after solving");
				}

				ast::ControlNode * childNode = processRegion(false, region, analysis, visited);

				children[region.getHeader()] = childNode;
			}

			IF_DEBUG {
				llvm::errs() << "checking on the session . . \n";
				ifBuilder->dump();
			}

			// build node
			oNode = ifBuilder->build(children, exitBlock);

			if (exitBlock == context.exitBlock)
				return 0;
			else
				return exitBlock;
		}

		Log::fail(termInst, "unsupported control-flow detected");
	}

	ast::FunctionNode * RestructuringPass::runOnFunction(llvm::Function & func)
	{
		AnalysisStruct A;
		rebuildAnalysisStruct(func, A);

		BlockSet visited;

#ifdef DEBUG_VIEW_CFGS
		llvm::errs() << "VIEW_CFG: original CFG\n";
		func.viewCFGOnly();
#endif

		IF_DEBUG {
				llvm::errs() << "Restruct: begin dump \n";
				func.dump();
				llvm::errs() << "Restruct: end dump\n";
				llvm::errs() << "### LoopInfo ###\n";
				A.getLoopInfo().print(llvm::errs());
				llvm::errs() << "### DomTree ###\n";
				A.getDomTree().print(llvm::errs());
				llvm::errs() << "### PostDomTree ###\n";
				A.getPostDomTree().dump();
				// func.viewCFGOnly();
		}


		llvm::BasicBlock * bb = &( func.getEntryBlock() );
		ExtractorContext context;

		ExtractorRegion bodyRegion(bb, context);
		ast::ControlNode * body = processRegion(false, bodyRegion, A, visited);
		return ast::ASTFactory::createFunction(&func, body);
	}

	void  RestructuringPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		usage.addRequired<llvm::PostDominatorTree>();
		usage.addRequired<llvm::DominatorTreeWrapperPass>();
		usage.addRequired<llvm::LoopInfoWrapperPass>();
		usage.addRequired<ScalarEvolutionWrapperPass>();
	}

	bool RestructuringPass::runOnModule(llvm::Module & M)
	{
#ifdef DEBUG_PASSRUN
		llvm::errs() << "\n\n##### PASS: Restructuring Pass #####\n\n";
		verifyModule(M);
#endif
		for (llvm::Module::iterator func = M.begin(); func != M.end(); ++func)
		{
			if (!func->isDeclaration()) {
				ast::FunctionNode * funcNode = runOnFunction(*func);
				IF_DEBUG {
					errs() << "Restructured CFG of function " << func->getName() << "\n";
					funcNode->dump();
				}
				astMap[(llvm::Function*) func] = funcNode;
			}
		}

		return true;
	}

	const char *  RestructuringPass::getPassName() const
	{
		return "axtor - modular AST extraction pass";
	}


	void RestructuringPass::releaseMemory()
	{
		for (ast::ASTMap::iterator pair = astMap.begin(); pair != astMap.end(); ++pair)
		{
			delete pair->second;
		}

		astMap.clear();
	}

	const ast::ASTMap & RestructuringPass::getASTs() const
	{
		return astMap;
	}

}

