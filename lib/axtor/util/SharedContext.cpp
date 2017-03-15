/*
 * SharedContext.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/util/SharedContext.h>

#include <llvm/IR/LLVMContext.h>

namespace axtor {

llvm::LLVMContext * SharedContext::context = NULL;

void SharedContext::init(llvm::LLVMContext & axtorContext)
{
    context = &axtorContext;
}

llvm::LLVMContext & SharedContext::get()
{
  assert(context && "axtor not initialized properly");
  return *context;
}

}
