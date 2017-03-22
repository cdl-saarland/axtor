/*
 * Axtor.h
 *
 *  Created on: 28.03.2010
 *      Author: Simon Moll
 */

#ifndef AXTOR_HPP_
#define AXTOR_HPP_

#include <axtor/backend/AxtorBackend.h>
#include <axtor/metainfo/ModuleInfo.h>

namespace llvm {
	namespace legacy {
		class PassManager;
	}
}

namespace axtor {

/*
 * initializes the library
 */
void initialize(bool alsoLLVM, llvm::LLVMContext * userContext = nullptr);

/*
 * translates the module described by modinfo using the corresponding backend
 */
void translateModule(AxtorBackend & backend, ModuleInfo & modInfo);

/*
 * adds all required passes to @pm for translating the module
 */
void addBackendPasses(AxtorBackend & backend, ModuleInfo & modInfo, llvm::legacy::PassManager & pm);

}

#endif /* AXTOR_HPP_ */
