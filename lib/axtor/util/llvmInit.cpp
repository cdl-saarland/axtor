/*
 * llvmInit.cpp
 *
 *  Created on: 09.04.2011
 *      Author: gnarf
 */
#include <axtor/util/llvmInit.h>

#include <llvm/Support/ManagedStatic.h>
#include <llvm/PassRegistry.h>
#include <llvm/InitializePasses.h>


namespace axtor {

void
initLLVM() {
	llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
	llvm::initializeCore(Registry);
	llvm::initializeScalarOpts(Registry);
	llvm::initializeIPO(Registry);
	llvm::initializeAnalysis(Registry);
	llvm::initializeIPA(Registry);
	llvm::initializeTransformUtils(Registry);
	llvm::initializeInstCombine(Registry);
	llvm::initializeInstrumentation(Registry);
	llvm::initializeTarget(Registry);
}

} /* namespace axtor */
