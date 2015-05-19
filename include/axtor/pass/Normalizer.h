/*  Axtor - AST-Extractor for LLVM
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Normalizer.h
 *
 *  Created on: Jul 26, 2010
 *      Author: Simon Moll
 */

#ifndef NORMALIZER_HPP_
#define NORMALIZER_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>

namespace llvm {
	class Module;
	class Function;
	class BasicBlock;
	class Loop;
	class LoopInfo;
}

namespace axtor {

typedef std::vector<std::pair<llvm::BasicBlock*, llvm::BasicBlock*> > EdgeVector;

/*
 * This pass applies node splitting to restructure the acyclic portions of all function CFGs
 */
class Normalizer : public llvm::ModulePass
{
	static char ID;

	bool normalizeSubLoopGraphs(llvm::Function & func, llvm::Loop * loop);

	bool normalizePostDomSubgraphs(llvm::Function & func, llvm::BasicBlock * entry, llvm::BasicBlock * barrierBlock, llvm::Loop * loopScope);

	bool normalizeNode(llvm::Function & func, llvm::BasicBlock * entry, llvm::BasicBlock * entryPostDom, llvm::Loop * loopScope);

	EdgeVector getAbstractSuccessors(llvm::LoopInfo& loopInfo, llvm::BasicBlock * entry, llvm::Loop * loopScope);

public:
	Normalizer();
	virtual ~Normalizer();

	virtual bool runOnModule(llvm::Module & M);
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
};

}

#endif /* NORMALIZER_HPP_ */
