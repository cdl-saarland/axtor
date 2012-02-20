/*
 * llvmDomination.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/util/llvmDomination.h>

namespace axtor {

BlockSet getSelfDominatedBlocks(llvm::BasicBlock * entryBlock, llvm::PostDominatorTree & postDomTree)
{
	assert(entryBlock && "invalid call");

	llvm::BasicBlock * block = entryBlock;
	typedef llvm::GraphTraits<llvm::BasicBlock*> CFG;

	BlockSet result;
	BlockVector pending;

	do
	{
		for(CFG::ChildIteratorType itChild = CFG::child_begin(block); itChild != CFG::child_end(block); ++itChild)
		{
			llvm::BasicBlock * next = *itChild;

			if (result.find(next) == result.end()) {
				if (postDomTree.dominates(entryBlock, next)) {
					//std::cerr << "pushing " << block->getName().str() << "\n";
					result.insert(next);
					pending.push_back(next);
				}
			}
		}

		//fetch
		if (pending.empty()) {
			block = NULL;
		} else {
			block = pending.back();
			pending.pop_back();
		}
	} while (block);

	return result;
}

bool dominatesAll(llvm::DominatorTree & domTree, llvm::DomTreeNode * node, const BlockSet & blocks)
{
	for(BlockSet::const_iterator itBlock = blocks.begin(); itBlock != blocks.end(); ++itBlock)
	{
		if (! domTree.dominates(node, domTree.getNode(*itBlock)))
			return false;
	}
	return true;
}

llvm::DomTreeNode * findImmediateDominator(llvm::DominatorTree & domTree, const BlockSet & blocks)
{
	llvm::DomTreeNode * node = domTree.getRootNode();

	for (DomTreeNodeVector::const_iterator itChildNode = node->getChildren().begin(); itChildNode != node->getChildren().end();)
	{
		llvm::DomTreeNode * childNode = *itChildNode;
		//descent
		if (dominatesAll(domTree, childNode, blocks)) {
			node = childNode;
			itChildNode = node->getChildren().begin();

		// try next
		} else {
			++itChildNode;
		}
	}

	return node;
}

}
