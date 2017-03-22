/*
 * TargetProvider.h
 *
 *  Created on: 22.04.2010
 *      Author: Simon Moll
 */

#ifndef TARGETPROVIDER_HPP_
#define TARGETPROVIDER_HPP_

#include <axtor/config.h>

#include <llvm/Pass.h>

#include <axtor/backend/AxtorBackend.h>
#include <axtor/metainfo/ModuleInfo.h>
#include <axtor/util/BlockCopyTracker.h>

namespace axtor {

struct TargetProvider : public llvm::ImmutablePass
{
	static char ID;

private:
	AxtorBackend * backend;
	ModuleInfo * modInfo;

public:
	TargetProvider(AxtorBackend & _backend, ModuleInfo & _modInfo) :
		llvm::ImmutablePass(ID),
		backend(&_backend),
		modInfo(&_modInfo)
	{
		assert(backend->hasValidType(modInfo) && "invalid ModuleInfo class for this backend");
#ifdef DEBUG
		std::cerr << "### INPUT MODULE ###\n";
		modInfo->dumpModule();
		std::cerr << "[EOF]\n";
#endif
	}


	TargetProvider() :
		llvm::ImmutablePass(ID),
		backend(NULL),
		modInfo(NULL)
	{
		assert(false && "invalid ctor");
	}

	virtual void initializePass();

	AxtorBackend & getBackend() const;

	ModuleInfo & getModuleInfo() const;

	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;

	//FIXME deprecated functionallity
	//ModuleInfo & getTracker() const;
};

}

#endif /* TARGETPROVIDER_HPP_ */
