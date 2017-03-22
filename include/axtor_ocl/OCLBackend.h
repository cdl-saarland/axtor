/*
 * OCLBackend.h
 *
 *  Created on: 07.04.2010
 *      Author: Simon Moll
 */

#ifndef OCLBACKEND_HPP_
#define OCLBACKEND_HPP_

#include <axtor/backend/AxtorBackend.h>
#include <axtor/intrinsics/PlatformInfo.h>

#include "OCLWriter.h"
#include "OCLModuleInfo.h"


namespace axtor {

class OCLBackend : public AxtorBackend
{
	static PlatformInfo * platform;

	static void init();

public:
	OCLBackend();

	virtual bool hasValidType(ModuleInfo * modInfo);

	virtual const std::string & getName();
	virtual const std::string & getLLVMDataLayout();

	static const std::string & getNameString();
	static const std::string & getLLVMDataLayoutString();

	//factory methods
	virtual SyntaxWriter * createModuleWriter(ModuleInfo & modInfo, const IdentifierScope & globals);

	virtual SyntaxWriter * createFunctionWriter(SyntaxWriter * modWriter, llvm::Function * func);

	virtual SyntaxWriter * createBlockWriter(SyntaxWriter * writer);

	virtual bool implementsFunction(llvm::Function * func);

	//does suppress binding for the global_fake_sampler call
	virtual bool requiresDesignator(llvm::Instruction * inst);

	//interface for specifying passes specific to this backend
	virtual void getAnalysisUsage(llvm::AnalysisUsage & usage) const;
	virtual void addRequiredPasses(llvm::legacy::PassManager & pm) const;
};

}

#endif /* OCLBACKEND_HPP_ */
