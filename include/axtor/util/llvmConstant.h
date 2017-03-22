/*
 * llvmConstant.h
 *
 *  Created on: 05.03.2010
 *      Author: Simon Moll
 */

#ifndef LLVMCONSTANT_HPP_
#define LLVMCONSTANT_HPP_

#include <string>

#include <llvm/ADT/APInt.h>

#include <axtor/config.h>
#include <axtor/util/SharedContext.h>

#include <llvm/IR/Constants.h>

namespace llvm {
	class LLVMContext;
	class IntegerType;
	class Module;
}

namespace axtor {

llvm::ConstantInt * get_uint(uint val);

llvm::ConstantInt * get_uint(uint val, llvm::LLVMContext & context, llvm::IntegerType * type);

llvm::ConstantInt * get_int(int val);

llvm::ConstantInt * get_int(uint64_t val, int bits);

llvm::ConstantInt * get_int(int val, llvm::LLVMContext & context, llvm::IntegerType * type);

llvm::Constant * get_stringGEP(llvm::Module * module, std::string content);

bool evaluateString(llvm::Value * val, std::string & out);

bool evaluateInt(llvm::Value * val, uint64_t & oValue);

}

#endif /* LLVMCONSTANT_HPP_ */
