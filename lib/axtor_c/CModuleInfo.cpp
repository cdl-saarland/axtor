/*
 * CModuleInfo.cpp
 *
 *  Created on: 22.05.2015
 *      Author: Simon
 */

#include <axtor_c/CModuleInfo.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/IR/Module.h>
#include <axtor/util/llvmDebug.h>

#include "llvm/IR/Metadata.h"
#include <llvm/IR/TypeFinder.h>

using namespace llvm;

namespace axtor {

CModuleInfo::CModuleInfo(llvm::Module * mod,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{}

std::ostream & CModuleInfo::getStream()
{
	return out;
}

void CModuleInfo::dump()
{
	std::cerr << "C module descriptor\n";
}

void CModuleInfo::dumpModule()
{
	mod->dump();
}

IdentifierScope CModuleInfo::createGlobalBindings()
{
	ConstVariableMap globals;

	for(GlobalVariable & gv : mod->globals()) {
		std::string name = gv.getName();
		globals[&gv] = VariableDesc(&gv, name);
	}

#ifdef DEBUG
	std::cerr << "ModuleContext created\n";
#endif

	return IdentifierScope(globals);
}

/*
 * checks whether all types are supported and
 */
void CModuleInfo::verifyModule()
{
/*		for (llvm::Module::iterator func = getModule()->begin(); func != getModule()->end(); ++func)
	{
		//check function arguments first
		for (llvm::Function::iterator block = func->begin(); block != func->end(); ++block)
		{
			for(llvm::BasicBlock::iterator inst = block->begin(); inst != block->end; ++block) {

			}

		}
	} */
	axtor::verifyModule(*mod);
}

bool CModuleInfo::isTargetModule(llvm::Module * other) const
{
	return mod == other;
}

/*llvm::TypeSymbolTable & CModuleInfo::getTypeSymbolTable()
{
	return mod->getTypeSymbolTable();
}*/

llvm::Module * CModuleInfo::getModule()
{
	return mod;
}

void CModuleInfo::runPassManager(llvm::legacy::PassManager & pm)
{
	pm.run(*getModule());
}

}
