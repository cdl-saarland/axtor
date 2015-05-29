/*
 * OCLModuleInfo.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor_ocl/OCLModuleInfo.h>

#include <llvm/IR/Module.h>
#include <axtor/util/llvmDebug.h>

#include "llvm/IR/Metadata.h"
#include <llvm/IR/TypeFinder.h>

using namespace llvm;

namespace axtor {

bool OCLModuleInfo::scanForDoubleType()
{
	TypeFinder finder;
	finder.run(*mod, false);

	for (auto * type : finder) {

		if (type->getTypeID() == llvm::Type::DoubleTyID)
			return true;
	}


	return false;
}

bool OCLModuleInfo::requiresDoubleType()
{
	return usesDoubleType;
}

OCLModuleInfo::OCLModuleInfo(llvm::Module * mod,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{
	usesDoubleType = scanForDoubleType();
}

OCLModuleInfo::OCLModuleInfo(llvm::Module * mod,
                             FunctionVector kernels,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{
	usesDoubleType = scanForDoubleType();
	ValueVector kernelValues; kernelValues.reserve(kernels.size());

	for (FunctionVector::iterator itFun = kernels.begin(); itFun != kernels.end(); ++itFun)
		kernelValues.push_back(*itFun);

	encodeKernelsAsMetadata(mod, kernelValues);
}

void OCLModuleInfo::encodeKernelsAsMetadata(Module * mod, const ValueVector & kernels)
{
	assert(mod && "no module specified");
	usesDoubleType = scanForDoubleType();

	llvm::LLVMContext & context = mod->getContext();
	NamedMDNode * kernelMD = mod->getOrInsertNamedMetadata("opencl.kernels");
	std::vector<Metadata*> mdVec;
	for (ValueVector::const_iterator itKernel = kernels.begin(); itKernel != kernels.end(); ++itKernel) {
		llvm::Value * kernelVal = *itKernel;
		llvm::Metadata * kernelEntryMD = llvm::ValueAsMetadata::get(kernelVal);
		mdVec.push_back(kernelEntryMD);
	}

	kernelMD->addOperand(MDNode::get(context, mdVec));
}

std::ostream & OCLModuleInfo::getStream()
{
	return out;
}

bool OCLModuleInfo::isKernelFunction(llvm::Function *function)
{
  if(!function)
    return false;
  llvm::NamedMDNode *openCLMetadata = mod->getNamedMetadata("opencl.kernels");
  if(!openCLMetadata)
    return false;

  llvm::MDNode * kernelListNode = openCLMetadata->getOperand(0);
  for(unsigned K = 0, E = kernelListNode->getNumOperands(); K != E; ++K) {
    ValueAsMetadata * kernelMD = cast<ValueAsMetadata>(kernelListNode->getOperand(K));
	if(kernelMD->getValue() == function)
		return true;
  }
  return false;
}

FunctionVector OCLModuleInfo::getKernelFunctions()
{
	FunctionVector kernels;

	llvm::NamedMDNode *openCLMetadata = mod->getNamedMetadata("opencl.kernels");
	  if(!openCLMetadata)
	    return kernels;

	  openCLMetadata->dump();
	  if (openCLMetadata->getNumOperands() < 1)
		  return kernels;

	  llvm::MDNode * kernelListNode = openCLMetadata->getOperand(0);

	  for(unsigned K = 0, E = kernelListNode->getNumOperands(); K != E; ++K) {
	    llvm::ValueAsMetadata * kernelMD = cast<ValueAsMetadata>(kernelListNode->getOperand(K));
    	kernels.push_back(llvm::cast<llvm::Function>(kernelMD->getValue()));
	  }
	return kernels;
}

void OCLModuleInfo::dump()
{
	std::cerr << "OpenCL shader module descriptor\n"
			      << "kernel functions:\n";
	FunctionVector kernels = getKernelFunctions();
  for(std::vector<llvm::Function*>::const_iterator kernel = kernels.begin(), 
      end = kernels.end();
      kernel != end; ++kernel)
    std::cerr << "* " << (*kernel)->getName().str() << "\n";
}

void OCLModuleInfo::dumpModule()
{
	mod->dump();
}

IdentifierScope OCLModuleInfo::createGlobalBindings()
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
void OCLModuleInfo::verifyModule()
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

bool OCLModuleInfo::isTargetModule(llvm::Module * other) const
{
	return mod == other;
}

/*llvm::TypeSymbolTable & OCLModuleInfo::getTypeSymbolTable()
{
	return mod->getTypeSymbolTable();
}*/

llvm::Module * OCLModuleInfo::getModule()
{
	return mod;
}

void OCLModuleInfo::runPassManager(llvm::PassManager & pm)
{
	pm.run(*getModule());
}

}
