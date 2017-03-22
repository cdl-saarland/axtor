/*
 * PassThroughWriter.h
 *
 *  Created on: 03.05.2010
 *      Author: Simon Moll
 */

#ifndef PASSTHROUGHWRITER_HPP_
#define PASSTHROUGHWRITER_HPP_

#include "SyntaxWriter.h"

#include <axtor/CommonTypes.h>

namespace axtor {

class PassThroughWriter : public SyntaxWriter
{
	SyntaxWriter & writer;

public:
	PassThroughWriter(SyntaxWriter & _writer);
	virtual ~PassThroughWriter();

	virtual void dump();
	virtual void print(std::ostream & out);

	//### Prologues ###
	virtual void writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals); //should declare all variables in funcContext

	//### Declarations ###
	virtual void writeVariableDeclaration(const VariableDesc & desc);
	virtual void writeFunctionDeclaration(llvm::Function * func, IdentifierScope * locals = NULL);

	virtual void writeFunctionHeader(llvm::Function * func, IdentifierScope * locals = NULL);

	//### Control Flow Elements ##
	virtual void writeIf(const llvm::Value * condition, bool negateCondition, IdentifierScope & locals);
	virtual void writeElse();
	virtual void writeLoopContinue();
	virtual void writeLoopBreak();
	virtual void writeDo();


	//### Loops ###
	virtual void writeInfiniteLoopBegin();
	virtual void writeInfiniteLoopEnd();

	virtual void writePostcheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate);

	virtual void writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate, InstructionSet * oExpressionInsts);

	void writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate)
	{
	   return writePrecheckedWhile(branchInst, locals, negate, NULL);
	}

	//### Instructions ###
	virtual void writeAssign(const VariableDesc & dest, const VariableDesc & src);
	virtual void writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals);
	virtual void writeInstruction(const VariableDesc * desc, llvm::Instruction * inst, IdentifierScope & locals);
};

}

#endif /* PASSTHROUGHWRITER_HPP_ */
