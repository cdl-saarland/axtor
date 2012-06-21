/*  Axtor - AST-Extractor for LLVM
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
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
