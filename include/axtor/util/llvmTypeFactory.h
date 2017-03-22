/*
 * llvmTypeFactory.h
 *
 *  Created on: 27.02.2010
 *      Author: Simon Moll
 */

#ifndef LLVMTYPEFACTORY_HPP_
#define LLVMTYPEFACTORY_HPP_

#include <stdio.h>
#include <string.h>

#include <axtor/util/stringutil.h>
#include <axtor/util/SharedContext.h>

namespace llvm {
	class Type;
}

namespace axtor {

llvm::Type * generateType(char * data, char ** oData);

llvm::Type * generateType(const char * in);

}

#endif /* LLVMTYPEFACTORY_HPP_ */
