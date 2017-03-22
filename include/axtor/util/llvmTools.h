/*
 * llvmTools.h
 *
 *  Created on: 07.02.2010
 *      Author: Simon Moll
 */

#ifndef LLVMTOOLS_HPP_
#define LLVMTOOLS_HPP_


#include <iostream>

#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>

#include <axtor/util/SharedContext.h>

namespace axtor {

	llvm::Module* createModuleFromFile(std::string fileName);

	llvm::Module* createModuleFromFile(std::string fileName, llvm::LLVMContext & context);

	void writeModuleToFile(llvm::Module * M, const std::string & fileName);

}

#endif /* LLVMTOOLS_HPP_ */
