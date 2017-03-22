/*
 * LLVMGraphBuilder.h
 *
 *  Created on: 19.02.2010
 *      Author: Simon Moll
 */

#ifndef LLVMGRAPHBUILDER_HPP_
#define LLVMGRAPHBUILDER_HPP_

#include <llvm/Module.h>
#include <llvm/Function.h>

#include "LLVMGraphTypes.h"

/*
 * translates the CFG of a function into a generic graph
 */
static FunctionGraph buildGraphFromFunction(llvm::Function * func)
{
	assert(func && "was NULL");

	FunctionGraph::NodeVector nodes;

	//set up nodes
	for(llvm::Function::iterator bb = func->begin(); bb != func->end(); ++bb)
	{
		std::cerr << "pushed bb " << bb->getName().str() << '\n';
		FunctionGraph::GraphNode * node = new FunctionGraph::GraphNode(bb);
		nodes.push_back(node);
	}

	FunctionGraph graph(nodes, &( func->getEntryBlock() ));

	//set up edges
	for(uint i = 0; i < nodes.size(); ++i)
	{
		FunctionGraph::GraphNode * node = nodes[i];
		llvm::BasicBlock * block = node->getLabel();
		llvm::TerminatorInst * termInst = block->getTerminator();

		for(uint succ = 0; succ <  termInst->getNumSuccessors(); ++succ)
		{
			llvm::BasicBlock * succBlock = termInst->getSuccessor(succ);
			FunctionGraph::GraphNode * next = graph.getNodeByLabel(succBlock);
			node->addSuccessor(next);
		}
	}

	return graph;
}


#endif /* LLVMGRAPHBUILDER_HPP_ */
