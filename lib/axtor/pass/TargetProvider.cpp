/*
 * TargetProvider.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor/pass/TargetProvider.h>
#include <axtor/util/llvmDebug.h>

namespace axtor {
	char TargetProvider::ID = 0;

	llvm::RegisterPass<TargetProvider> __regTargetProvider("provider", "axtor - target backend and module information pass",
						true,
						true);



	void TargetProvider::initializePass() {}

	AxtorBackend & TargetProvider::getBackend() const
	{
		assert(backend && "was not properly initialized");
		return *backend;
	}

	ModuleInfo & TargetProvider::getModuleInfo() const
	{
		assert(modInfo && "was not properly initialized");
		return *modInfo;
	}

	 void TargetProvider::getAnalysisUsage(llvm::AnalysisUsage & usage) const
	{
		getBackend().getAnalysisUsage(usage);
	}

	 //FIXME
#if 0
	 BlockCopyTracker & TargetProvider::getTracker() const
	 {
		 assert(modInfo && "was not properly initialized");
		 return *modInfo;
	 }
#endif
}
