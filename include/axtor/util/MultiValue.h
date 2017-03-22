/*
 * MultiValue.h
 *
 *  Created on: 16.03.2010
 *      Author: Simon Moll
 */

#ifndef MULTIVALUE_HPP_
#define MULTIVALUE_HPP_

#include <algorithm>

#include <axtor/CommonTypes.h>

namespace axtor {

class MultiValue
{
public:
	typedef void (*ValueOperation)(llvm::Value * val);

private:
	class ValueFunctor : public std::unary_function<llvm::Value*,void>
	{
		ValueOperation op;
	public:
		ValueFunctor(ValueOperation _op);

		void operator()(llvm::Value * val);
	};

// ### FUNCTORS ###
	static void func_dropReferences(llvm::Value * val);

	static void func_erase(llvm::Value * val);

	static void func_removeConstantUsers(llvm::Value * val);

	static void func_dump(llvm::Value * val);

public:
	static void apply(ValueSet & values, ValueOperation op);

	static void erase(ValueSet & values);

	static void dump(ValueSet & values);
};

}


#endif /* MULTIVALUE_HPP_ */
