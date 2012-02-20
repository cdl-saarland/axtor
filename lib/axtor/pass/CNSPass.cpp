/*
 * Regularizer.cpp
 *
 *  Created on: 29.04.2010
 *      Author: gnarf
 */
#include <fstream>

#include <axtor/util/llvmDebug.h>
#include <axtor/util/ResourceGuard.h>
#include <axtor/pass/CNSPass.h>

namespace axtor {

char CNSPass::ID = 0;

llvm::RegisterPass<CNSPass> __regRegularizer("cns", "Controlled Node Splitting", true, false);

CNSPass::CNSPass() :
		llvm::ModulePass(ID)
{}

void CNSPass::getAnalysisUsage(llvm::AnalysisUsage & usage) const
{
	//usage.addRequired<llvm::DominatorTree>();
	usage.addRequired<TargetProvider>();
	usage.addPreserved<OpaqueTypeRenamer>();
}

/*
 * Controlled Node Splitting - main method
 */
cns::SplitTree * CNSPass::generateSplitSequence(cns::SplitTree * root, BlockGraph::SubgraphMask & mask, BlockGraph & graph)
{
#ifdef DEBUG
	std::cerr << "### regularize : "; dumpVector(mask);
#endif

	/*
	 * initial transformation (T1/T2)
	 */
	cns::minimizeGraph(mask, graph);

	cns::SplitTree * tree = root;

#ifdef DEBUG
	std::cerr << "graph Size after initial contraction=" << graph.getSize(mask) << "\n";
#endif

	while(graph.getSize(mask) > 1)
	{

		/*
		 * identify RC-nodes
		 */
#ifdef DEBUG
		std::cerr << "identifying candidate nodes (SED, non-RC). . \n";
#endif
		BlockGraph::SubgraphMask candidates = cns::detectCandidateNodes(mask, graph);

#ifdef DEBUG
			std::cerr << "candidate nodes: "; dumpVector(candidates);
#endif

		/*
		 * select splitting node (from the headers of the SCC)
		 */
		uint splitNode = cns::getLowestScoringNode(candidates, graph, &cns::scoreBranches);

#ifdef DEBUG
			std::cerr << "heuristic picked node: " << splitNode << "\n";
#endif
		/*
		 * split (complete graph mask gets modified to match)
		 */
		BlockGraph splitGraph = graph.createSplitGraph(mask, splitNode);

		tree = tree->pushSplit(mask, splitGraph, splitNode);

#ifdef DEBUG
		std::cerr << "graph after split";
		splitGraph.dump(mask);
		std::cerr << "tree:\n";
		tree->dump();
#endif

		//for now just iteratively operate on a single graph
		graph = splitGraph;

		/*
		 * compute limit graph
		 */
		cns::minimizeGraph(mask, graph);
	}

	return tree;
}

bool CNSPass::runOnModule(llvm::Module & M)
{
	TargetProvider & target = getAnalysis<TargetProvider>();
	BlockCopyTracker & tracker = target.getTracker();

#ifdef DEBUG_PASSRUN
		std::cerr << "\n\n##### PASS: CNS #####\n\n";
#endif

	bool changed = false;

	for(llvm::Module::iterator func = M.begin(); func != M.end(); ++func)
		if (! func->isDeclaration())
			changed |= runOnFunction(tracker, *func);

#ifdef DEBUG
	tracker.dump();
	verifyModule(M);
#endif

	return changed;
}

bool CNSPass::runOnFunction(BlockCopyTracker & tracker, llvm::Function & func)
{

	BlockGraph graph(func);
	BlockGraph::SubgraphMask mask = graph.createMask();
/*
	{
		std::ofstream of( (func.getNameStr() + "_graph.gv").c_str(), std::ios::out);
		graph.dumpGraphviz(mask, of);
	}
*/
	cns::SplitTree * root = new cns::SplitTree(mask, graph);
	cns::SplitTree * tree = generateSplitSequence(root, mask, graph);

#ifdef DEBUG
	tree->dump();
#endif

	uint length = tree->getDepth();
	std::vector<uint> nodes(length);

	//recover split sequence
	for(uint i = length; i > 0; --i)
	{
		nodes[i - 1] = tree->getSplitNode();
		tree = tree->getParent();
	}

	delete root;

	applySplitSequence(tracker, graph, nodes);

#ifdef DEBUG
	std::cerr << "regularized function : \n";
	func.dump();
#endif

	return true;
}

const char * CNSPass::getPassName() const
{
	return "Controlled Node Splitting pass";
}

void CNSPass::applySplitSequence(BlockCopyTracker & tracker, BlockGraph & graph, std::vector<uint> nodes) const
{
	for(uint i = 0; i < nodes.size(); ++i)
	{
		uint node = nodes[i];
		llvm::BasicBlock * splitBlock = graph.getLabel(node);

		splitNode(tracker, splitBlock);
	}
}


}
