/*
 * SyntaxWriter.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef SYNTAXWRITER_HPP_
#define SYNTAXWRITER_HPP_

#include <axtor/CommonTypes.h>
#include <axtor/metainfo/ModuleInfo.h>
#if 0
#include <llvm/Analysis/ScalarEvolution.h>
#endif

namespace axtor {

class ForLoopInfo; // util/llvmLoop.h

/*
 * SyntaxWriter - interface (used by the AST-extraction core during translation)
 */
struct SyntaxWriter
{
	virtual ~SyntaxWriter() {}

	virtual void dump() = 0;
	virtual void print(std::ostream & out) = 0;

	//### Prologues ###
	virtual void writeFunctionPrologue(llvm::Function * func, IdentifierScope & locals)=0; //should declare all variables in locals

	//### Declarations ###
	virtual void writeVariableDeclaration(const VariableDesc & desc) = 0;
	virtual void writeFunctionDeclaration(llvm::Function * func, IdentifierScope * locals = NULL) = 0;

	virtual void writeFunctionHeader(llvm::Function * func, IdentifierScope * locals = NULL) = 0;

	//### Control Flow Elements ##
	virtual void writeIf(const llvm::Value * condition, bool negateCondition, IdentifierScope & locals)=0;
	virtual void writeElse()=0;
	virtual void writeLoopContinue()=0;
	virtual void writeLoopBreak()=0;
	virtual void writeDo()=0;


	//### InfiniteLoop ###
	virtual void writeInfiniteLoopBegin()=0;
	virtual void writeInfiniteLoopEnd()=0;

	// For loop
	virtual void writeForLoopBegin(ForLoopInfo & forLoopInfo, IdentifierScope & locals) = 0;
#if 0
	virtual void writeForLoopEnd() = 0;
#endif
	virtual void writePostcheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate)=0;

	virtual void writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate, InstructionSet * oExpressionInsts)=0;

	void writePrecheckedWhile(llvm::BranchInst * branchInst, IdentifierScope & locals, bool negate)
	{
	   return writePrecheckedWhile(branchInst, locals, negate, NULL);
	}

	//### Instructions ###
	virtual void writePHIAssign(llvm::PHINode & phi, llvm::BasicBlock * incomingBlock, IdentifierScope & locals) = 0;

	virtual void writeAssignRaw(const std::string & destName, llvm::Value * val, IdentifierScope & locals)=0;
	virtual void writeAssign(const VariableDesc & dest, const VariableDesc & src)=0;
	virtual void writeAssignRaw(const std::string & dest, const std::string & src)=0;
	virtual void writeReturnInst(llvm::ReturnInst * retInst, IdentifierScope & locals)=0;
	virtual void writeInstruction(const VariableDesc * desc, llvm::Instruction * inst, IdentifierScope & locals) = 0;
};

}

#endif /* SYNTAXWRITER_HPP_ */
