/*
 * OCLModuleInfo.cpp
 *
 *  Created on: 25.04.2010
 *      Author: gnarf
 */

#include <axtor_ocl/OCLModuleInfo.h>

#include <axtor/util/llvmDebug.h>

#include "llvm/Metadata.h"
#include <llvm/Analysis/FindUsedTypes.h>

namespace axtor {

bool OCLModuleInfo::scanForDoubleType()
{
	typedef llvm::SetVector<llvm::Type*> TypeSetVector;
	typedef TypeSetVector::const_iterator TSV_Iterator;

	llvm::PassManager PM;
	llvm::FindUsedTypes * findTypesPass = new llvm::FindUsedTypes();
	PM.add(findTypesPass);
	PM.run(*mod);

	const TypeSetVector & typeVec = findTypesPass->getTypes();

	for (TypeSetVector::const_iterator type =typeVec.begin(); type!=typeVec.end(); ++type)
	{

		if (((llvm::Type*) *type)->getTypeID() == llvm::Type::DoubleTyID)
			return true;
	}


	return false;
}

bool OCLModuleInfo::requiresDoubleType()
{
	return usesDoubleType;
}

OCLModuleInfo::OCLModuleInfo(llvm::Module * mod,
                             llvm::Function * kernel,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{
	ValueVector kernelValues(1, kernel);
	init(kernelValues);
}


OCLModuleInfo::OCLModuleInfo(llvm::Module * mod,
                             FunctionVector kernels,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{
	ValueVector kernelValues; kernelValues.reserve(kernels.size());

	for (FunctionVector::iterator itFun = kernels.begin(); itFun != kernels.end(); ++itFun)
		kernelValues.push_back(*itFun);

	init(kernelValues);
}

OCLModuleInfo::OCLModuleInfo(llvm::Module * mod,
                             ValueVector kernels,
                             std::ostream & out) : ModuleInfo(*mod),
                                                   mod(mod),
                                                   out(out)
{
	init(kernels);
}

void OCLModuleInfo::init(const ValueVector & kernels)
{
	assert(mod && "no module specified");
	usesDoubleType = scanForDoubleType();

	llvm::LLVMContext & context = mod->getContext();
	llvm::NamedMDNode * kernelMD = mod->getOrInsertNamedMetadata("opencl.kernels");

	for (ValueVector::const_iterator itKernel = kernels.begin(); itKernel != kernels.end(); ++itKernel) {
		llvm::Value * kernelVal = *itKernel;
		llvm::MDNode * kernelEntryMD = llvm::MDNode::get(context, llvm::ArrayRef<llvm::Value*>(kernelVal));
		kernelMD->addOperand(kernelEntryMD);
	}
}

/*
 * helper method for creating a ModuleInfo object from a module and a bind file
 */
OCLModuleInfo OCLModuleInfo::createTestInfo(llvm::Module *mod, 
                                            std::ostream &out)
{
	//llvm::Function * kernelFunc = mod->getFunction("compute");
  //assert(kernelFunc && "could not find \" compute \" - function");
  std::vector<llvm::Function*> kernels; 
  llvm::NamedMDNode *openCLMetadata = mod->getNamedMetadata("opencl.kernels");
  assert(openCLMetadata && "No kernels in the module");

  for(unsigned K = 0, E = openCLMetadata->getNumOperands(); K != E; ++K) {
    llvm::MDNode &kernelMD = *openCLMetadata->getOperand(K);
    kernels.push_back(llvm::cast<llvm::Function>(kernelMD.getOperand(0)));
  }

	return OCLModuleInfo(mod, kernels, out);
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

  for(unsigned K = 0, E = openCLMetadata->getNumOperands(); K != E; ++K) {
    llvm::MDNode * kernelMD = openCLMetadata->getOperand(K);
    if (kernelMD != 0 && kernelMD->getNumOperands() > 0) {
    	llvm::Value * op = kernelMD->getOperand(0);
    	if(op == function)
    		return true;
    }
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

	  for(unsigned K = 0, E = openCLMetadata->getNumOperands(); K != E; ++K) {
	    llvm::MDNode * kernelMD = openCLMetadata->getOperand(K);
	    if (kernelMD != 0 && kernelMD->getNumOperands() > 0) {
	    	llvm::Value * op = kernelMD->getOperand(0);

	    	if(op != 0 && llvm::isa<llvm::Function>(op))
	    		kernels.push_back(llvm::cast<llvm::Function>(kernelMD->getOperand(0)));
	    }
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

	for(llvm::Module::const_global_iterator it = mod->global_begin(); it != mod->global_end(); ++it)
	{
		if (llvm::isa<llvm::GlobalVariable>(it))
		{
			std::string name = it->getName().str();
			globals[it] = VariableDesc(it, name);
		}
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
