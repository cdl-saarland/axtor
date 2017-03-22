/*
 * ASTFactory.h
 *
 *  Created on: 12.06.2010
 *      Author: Simon Moll
 */

#ifndef ASTFACTORY_HPP_
#define ASTFACTORY_HPP_

#include "BasicNodes.h"

namespace axtor {
namespace ast {

struct ASTFactory
{
	static FunctionNode * createFunction(llvm::Function * _func, ControlNode * body);
	static ControlNode * createConditional(llvm::BasicBlock * block, ControlNode * _onTrue, ControlNode * _onFalse);

	static ControlNode * createBlock(llvm::BasicBlock * block);
	static ControlNode * createList(const NodeVector & nodes);

	// static ControlNode * createForLoop(llvm::PHINode * ivPHI, ControlNode * body);
	static ControlNode * createInfiniteLoop(ControlNode * child);
	static ControlNode * createBreak(llvm::BasicBlock * block);
	static ControlNode * createBreak();
	static ControlNode * createContinue(llvm::BasicBlock * block);
	static ControlNode * createContinue();

	static ControlNode * createReturn(llvm::BasicBlock * block);
	static ControlNode * createUnreachable(llvm::BasicBlock * block);
};

}
}

#endif /* ASTFACTORY_HPP_ */
