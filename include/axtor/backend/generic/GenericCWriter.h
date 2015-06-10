/*
 * GenericCWriter.h
 *
 *  Created on: Jun 8, 2012
 *      Author: simoll
 */

#ifndef GENERICCWRITER_H_
#define GENERICCWRITER_H_


#include <axtor/writer/SyntaxWriter.h>
#include <axtor/intrinsics/AddressIterator.h>
#include <axtor/CommonTypes.h>
#include <axtor/util/WrappedOperation.h>
#include <sstream>

#include <llvm/Analysis/ScalarEvolution.h>

namespace axtor {

/*
 * Implements generic tokens and basic facilities using interfaces for string streams
 *
 * This class implements stream independent functionality
 *
 * Generic stream sensitive functions are implemented in the GenericCSerializer
 */
class GenericCWriter : public SyntaxWriter
{
	friend class GenericCSerializer;
public:
	void writeElse();

	void writeLoopContinue();

	void writeLoopBreak();

	void writeDo();

	std::string getOperatorToken(const WrappedOperation & op, bool & isSigned);

	std::string buildArraySubscript(std::string root, AddressIterator *& address, IdentifierScope & locals);

	void writeAssignRaw(const std::string & dest, const std::string & src);
	void writeAssignRaw(const std::string & destName, llvm::Value * val, IdentifierScope & locals);

	std::string getPointerTo(llvm::Value * val, IdentifierScope & locals, const std::string * rootName = 0);
	std::string getReferenceTo(llvm::Value * val, IdentifierScope & locals, const std::string * rootName = 0);
	std::string getConstant(llvm::Constant * constant, IdentifierScope & locals);

	// needs to be implemented
 	virtual std::string unwindPointer(llvm::Value * val, IdentifierScope & locals, bool & oIsDereffed, const std::string * rootName) = 0;
	virtual std::string getNonInstruction(llvm::Value * op, IdentifierScope & locals) = 0;
	virtual std::string getOperation(const WrappedOperation & op, StringVector operands) = 0;
	virtual std::string getLiteral(llvm::Constant * const) = 0;




	inline void writeAssign(const VariableDesc & dest, const VariableDesc & src)
	{
	#ifdef DEBUG
		std::cerr << "writeAssign:: enforcing assignment to " << dest.name << std::endl;
	#endif
		writeAssignRaw(dest.name, src.name);
	}

protected:

	virtual void put(const std::string&)=0;

	inline void putLineBreak()
	{
		put ('\n');
	}

	inline void putLine(std::string text)
	{
		{
			put( text + '\n');
		}
	}

	inline void put(char c)
	{
		std::stringstream tmp;
		tmp << c;
		put( tmp.str() );
	}

	inline void putLine(char c)
	{
		std::stringstream tmp;
		tmp << c;
		putLine( tmp.str() );
	}
};

}

#endif /* GENERICCWRITER_H_ */
