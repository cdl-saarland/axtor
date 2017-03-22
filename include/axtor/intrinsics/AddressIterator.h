/*
 * AddressIterator.h
 *
 *  Created on: 19.03.2010
 *      Author: Simon Moll
 */

#ifndef ADDRESSITERATOR_HPP_
#define ADDRESSITERATOR_HPP_

#include <axtor/util/llvmConstant.h>
#include <axtor/CommonTypes.h>

namespace llvm {
	class Value;
}

namespace axtor {

/*
 * Auxiliary class
 */
class AddressIterator
{
	llvm::Value * val;
	AddressIterator * next;
	bool cumulativeValue;

	AddressIterator();
	AddressIterator(AddressIterator * _next, llvm::Value * _val, bool _isCumulative);


public:
	void dump(int idx);
	void dump();

	llvm::Value * getValue();

	AddressIterator * getNext();

	bool isEnd() const;

	bool isEmpty() const;

	// true, if this is an addition operating on a GEP (without dereferencing the type)
	// Note, that added zeros in cascading GEPs will be eliminated to begin with
	bool isCumulative() const;

	/*
	 * returns a pair consisting of the dereferenced root object and a corresponding index value list
	 */

	struct AddressResult
	{
		llvm::Value * rootValue;
		AddressIterator * iterator;

		AddressResult(llvm::Value * _rootValue, AddressIterator * _iterator);
	};

	static AddressResult createAddress(llvm::Value * derefValue, StringSet & derefFuncs);
};

}

#endif /* ADDRESSITERATOR_HPP_ */
