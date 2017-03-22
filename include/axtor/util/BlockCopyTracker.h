/*
 * BlockCopyTracker.h
 *
 *  Created on: 27.06.2010
 *      Author: Simon MOll
 */

#ifndef BLOCKCOPYTRACKER_HPP_
#define BLOCKCOPYTRACKER_HPP_

#include <map>

#include <axtor/CommonTypes.h>

namespace axtor {

class BlockCopyTracker
{
	typedef	const llvm::BasicBlock * ConstBlock;
	typedef std::map<ConstBlock,int> IndexMap;

	IndexMap indices;
	BlockVector originalBlocks;
	int getIndex(ConstBlock block) const;

public:
	BlockCopyTracker(llvm::Module & M);
	virtual ~BlockCopyTracker() {}

	//duplicate tracker functionality
	void identifyBlocks(ConstBlock first, ConstBlock second);
	bool equalBlocks(ConstBlock first, ConstBlock second) const;
	ConstBlockSet getEqualBlocks(const llvm::BasicBlock * block) const;
	void dump() const;

	llvm::BasicBlock * getOriginalBlock(const llvm::BasicBlock * block) const;
	bool isOriginalBlock(const llvm::BasicBlock * block) const;
};

}

#endif /* BLOCKCOPYTRACKER_HPP_ */
