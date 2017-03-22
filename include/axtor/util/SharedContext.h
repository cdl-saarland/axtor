/*
 * SharedContext.h
 *
 *  Created on: 27.02.2010
 *      Author: Simon Moll
 */

#ifndef SHAREDCONTEXT_HPP_
#define SHAREDCONTEXT_HPP_

#include <iostream>
#include <assert.h>

namespace llvm {
	class LLVMContext;
}

namespace axtor {

class SharedContext
{
	static llvm::LLVMContext * context;

public:
	static void init(llvm::LLVMContext & context);

	static llvm::LLVMContext & get();
};

}


#endif /* SHAREDCONTEXT_HPP_ */
