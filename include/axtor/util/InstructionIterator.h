/*
 * InstructionIterator.h
 *
 *  Created on: 13.04.2010
 *      Author: Simon Moll
 */

#ifndef INSTRUCTIONITERATOR_HPP_
#define INSTRUCTIONITERATOR_HPP_

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>

namespace axtor {

class InstructionIterator
{
	llvm::Module & mod;

	llvm::Module::iterator func;
	llvm::Function::iterator block;
	llvm::BasicBlock::iterator inst;

public:
	InstructionIterator(llvm::Module & _mod);

	bool finished() const;

	InstructionIterator operator++();

	llvm::Instruction * operator*();
};

}

#endif /* INSTRUCTIONITERATOR_HPP_ */
