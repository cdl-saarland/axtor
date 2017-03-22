/*
 * TypeHandler.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef TYPEHANDLER_HPP_
#define TYPEHANDLER_HPP_

#include <set>

#include <llvm/TypeSymbolTable.h>
#include <llvm/Instructions.h>


#include "AddressIterator.h"
#include "TypeStringBuilder.h"

#warning "deprecated"

namespace axtor {

struct TypeHandler
{
	/*
	 * return a name (debug)
	 */
	virtual std::string getName()=0;

	/*
	 * return a list of declarations that need to precede any function
	 */
	//virtual std::string getModuleEpilogue(llvm::TypeSymbolTable & symTable)=0;

	/*
	 * configure @builder so it evaluates to a name for this type
	 */
	virtual void getSymbol(TypeStringBuilder * builder, llvm::TypeSymbolTable & typeSymbols)=0;

	/*
	 * check whether this TypeHandler applies to to @type
	 */
	virtual bool appliesTo(const llvm::Type * type)=0;

	/*
 	 * register intrinsic descriptors with a platform
	 */
	virtual void registerWithPlatform(StringSet & nativeTypes, IntrinsicsMap & intrinsics, StringSet & derefFuncs)
	{}

	/*
	 * return a string built around @obj using values from @address
	 */
	virtual std::string dereference(std::string obj, const llvm::Type * objType, AddressIterator *& address, FunctionContext & funcContext, const llvm::Type* & oElementType)=0;
};

typedef std::set<TypeHandler*> TypeHandlerSet;

}

#endif /* TYPEHANDLER_HPP_ */
